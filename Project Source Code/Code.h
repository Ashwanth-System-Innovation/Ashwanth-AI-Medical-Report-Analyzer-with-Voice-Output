"""
AI-Driven Medical Imaging Analysis System for Multi-Modal Disease Detection
---------------------------------------------------------------------------
Created by: Ashwanth 
Version: 4.1.0

This system integrates computer vision, deep learning, and natural language processing
to analyze various medical imaging formats and provide diagnostic insights with
multilingual audio feedback.

Supported imaging formats:
- X-Ray
- MRI
- CT Scan
- ECG Reports
- Ultrasound
- Standard medical reports (text)

Hardware components:
- Modified document scanner connected via USB
- Orange Pi (processing unit)
- Custom button interface for patient interaction
- Audio output system
"""

import os
import sys
import time
import logging
import argparse
import numpy as np
import tensorflow as tf
import cv2
import torch
from PIL import Image
from transformers import AutoModelForImageClassification, ViTImageProcessor
from transformers import AutoModelForSeq2SeqLM, AutoTokenizer
import pytesseract
from gtts import gTTS
import pygame
import json
import requests
from concurrent.futures import ThreadPoolExecutor

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler("medical_imaging_system.log"),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger("MedicalImagingAI")

# Configuration
CONFIG = {
    "api_endpoint": "https://api.medicalimaging.ai/v1/analyze",
    "api_key": os.environ.get("MEDICAL_AI_API_KEY", "your_api_key_here"),
    "scanner_device": "/dev/ttyUSB0",
    "models_path": "./models",
    "temp_path": "./temp",
    "output_path": "./results",
    "supported_languages": ["english", "tamil", "malayalam"],
    "default_language": "english",
    "confidence_threshold": 0.75,
    "scan_resolution": 300,  # DPI
    "max_scan_size": (8.5, 14),  # inches
    "server_mode": False,
    "gpu_enabled": True,
    "batch_size": 1,
    "use_local_models": True,
    "use_api_fallback": True,
    "audio_volume": 0.8,
    "button_gpio_pin": 17,
    "led_status_pin": 27,
    "led_error_pin": 22
}

class MedicalImagingSystem:
    def __init__(self, config=None):
        """Initialize the Medical Imaging Analysis System"""
        self.config = config or CONFIG
        logger.info("Initializing Medical Imaging Analysis System...")
        
        # Create necessary directories
        os.makedirs(self.config["temp_path"], exist_ok=True)
        os.makedirs(self.config["output_path"], exist_ok=True)
        
        # Initialize hardware components
        self._init_hardware()
        
        # Load AI models
        self._init_models()
        
        # Initialize language processing
        self._init_language_processing()
        
        # Initialize audio system
        self._init_audio_system()
        
        logger.info("System initialization complete")
        
    def _init_hardware(self):
        """Initialize hardware components including scanner and GPIO"""
        logger.info("Initializing hardware components...")
        try:
            # Initialize scanner connection
            self.scanner = ScannerDevice(self.config["scanner_device"])
            logger.info(f"Scanner connected: {self.scanner.get_device_info()}")
            
            # Initialize GPIO for button and LEDs
            import RPi.GPIO as GPIO
            GPIO.setmode(GPIO.BCM)
            GPIO.setup(self.config["button_gpio_pin"], GPIO.IN, pull_up_down=GPIO.PUD_UP)
            GPIO.setup(self.config["led_status_pin"], GPIO.OUT)
            GPIO.setup(self.config["led_error_pin"], GPIO.OUT)
            
            # Set up button callback
            GPIO.add_event_detect(
                self.config["button_gpio_pin"], 
                GPIO.FALLING, 
                callback=self._button_callback, 
                bouncetime=300
            )
            
            # Test LEDs
            GPIO.output(self.config["led_status_pin"], GPIO.HIGH)
            time.sleep(0.5)
            GPIO.output(self.config["led_status_pin"], GPIO.LOW)
            GPIO.output(self.config["led_error_pin"], GPIO.HIGH)
            time.sleep(0.5)
            GPIO.output(self.config["led_error_pin"], GPIO.LOW)
            
            self.hardware_ready = True
            logger.info("Hardware initialization successful")
        except Exception as e:
            self.hardware_ready = False
            logger.error(f"Hardware initialization failed: {str(e)}")
            raise RuntimeError(f"Failed to initialize hardware: {str(e)}")
    
    def _init_models(self):
        """Initialize AI models for different imaging types"""
        logger.info("Loading AI models...")
        
        if not self.config["use_local_models"]:
            logger.info("Using API-only mode, skipping local model loading")
            return
        
        try:
            # X-Ray analysis model
            self.xray_processor = ViTImageProcessor.from_pretrained("medical-ai/xray-vit-base")
            self.xray_model = AutoModelForImageClassification.from_pretrained("medical-ai/xray-vit-base")
            
            # MRI analysis model
            self.mri_model = torch.hub.load('pytorch/vision:v0.10.0', 'resnet50', pretrained=True)
            self.mri_model.load_state_dict(torch.load(f"{self.config['models_path']}/mri_model.pth"))
            self.mri_model.eval()
            
            # CT scan analysis model
            self.ct_model = tf.keras.models.load_model(f"{self.config['models_path']}/ct_model")
            
            # ECG analysis model
            self.ecg_model = tf.keras.models.load_model(f"{self.config['models_path']}/ecg_model")
            
            # OCR for text reports
            self.ocr_lang_map = {
                "english": "eng",
                "tamil": "tam",
                "malayalam": "mal"
            }
            
            # General medical report analyzer
            self.report_tokenizer = AutoTokenizer.from_pretrained("medical-ai/medrpt-bert-base")
            self.report_model = AutoModelForSeq2SeqLM.from_pretrained("medical-ai/medrpt-bert-base")
            
            logger.info("AI models loaded successfully")
        except Exception as e:
            logger.error(f"Failed to load AI models: {str(e)}")
            if not self.config["use_api_fallback"]:
                raise RuntimeError(f"Failed to load AI models and API fallback is disabled: {str(e)}")
            else:
                logger.warning("Will use API fallback for analysis")
    
    def _init_language_processing(self):
        """Initialize NLP components for report analysis and translation"""
        logger.info("Initializing language processing components...")
        
        try:
            # Load medical terminology database
            with open(f"{self.config['models_path']}/medical_terminology.json", "r") as f:
                self.medical_terms = json.load(f)
            
            # Load translation models for supported languages
            self.translators = {}
            for lang in self.config["supported_languages"]:
                if lang != "english":
                    model_name = f"Helsinki-NLP/opus-mt-en-{lang}"
                    self.translators[lang] = {
                        "tokenizer": AutoTokenizer.from_pretrained(model_name),
                        "model": AutoModelForSeq2SeqLM.from_pretrained(model_name)
                    }
            
            logger.info("Language processing components initialized")
        except Exception as e:
            logger.error(f"Failed to initialize language processing: {str(e)}")
            raise RuntimeError(f"Language processing initialization failed: {str(e)}")
    
    def _init_audio_system(self):
        """Initialize audio output system"""
        logger.info("Initializing audio system...")
        
        try:
            pygame.mixer.init()
            pygame.mixer.music.set_volume(self.config["audio_volume"])
            
            # Pre-generate common system messages
            self._generate_system_audio_messages()
            
            logger.info("Audio system initialized")
        except Exception as e:
            logger.error(f"Failed to initialize audio system: {str(e)}")
            raise RuntimeError(f"Audio system initialization failed: {str(e)}")
    
    def _generate_system_audio_messages(self):
        """Pre-generate common system audio messages"""
        system_messages = {
            "welcome": "Welcome to the Medical Imaging Analysis System. Please place your document on the scanner and press the button.",
            "scanning": "Scanning your document. Please wait.",
            "analyzing": "Document scanned. Now analyzing the results.",
            "error": "An error occurred. Please try again.",
            "complete": "Analysis complete. I will now read the results."
        }
        
        self.system_audio = {}
        for lang in self.config["supported_languages"]:
            self.system_audio[lang] = {}
            for msg_key, msg_text in system_messages.items():
                if lang != "english":
                    # Translate message
                    msg_text = self._translate_text(msg_text, lang)
                
                # Generate audio file
                audio_file = f"{self.config['temp_path']}/system_{lang}_{msg_key}.mp3"
                tts = gTTS(text=msg_text, lang=self._get_gtts_lang_code(lang))
                tts.save(audio_file)
                self.system_audio[lang][msg_key] = audio_file
    
    def _get_gtts_lang_code(self, language):
        """Convert our language names to gTTS language codes"""
        lang_map = {
            "english": "en",
            "tamil": "ta",
            "malayalam": "ml"
        }
        return lang_map.get(language, "en")
    
    def _button_callback(self, channel):
        """Handle button press event"""
        logger.info("Button pressed, initiating scan and analysis")
        
        # Indicate processing with status LED
        import RPi.GPIO as GPIO
        GPIO.output(self.config["led_status_pin"], GPIO.HIGH)
        
        try:
            # Play audio notification
            self.play_system_audio("scanning")
            
            # Perform document scan
            scan_path = self.scan_document()
            
            # Play analyzing audio
            self.play_system_audio("analyzing")
            
            # Process the scanned document
            result = self.analyze_document(scan_path)
            
            # Play completion audio
            self.play_system_audio("complete")
            
            # Generate and play result audio
            self.generate_and_play_result(result)
            
            # Turn off status LED
            GPIO.output(self.config["led_status_pin"], GPIO.LOW)
            
        except Exception as e:
            logger.error(f"Error during scan and analysis: {str(e)}")
            
            # Indicate error with LED
            GPIO.output(self.config["led_status_pin"], GPIO.LOW)
            GPIO.output(self.config["led_error_pin"], GPIO.HIGH)
            
            # Play error audio
            self.play_system_audio("error")
            
            # Turn off error LED after delay
            time.sleep(3)
            GPIO.output(self.config["led_error_pin"], GPIO.LOW)
    
    def play_system_audio(self, message_key):
        """Play a pre-generated system audio message"""
        language = self.current_language or self.config["default_language"]
        audio_file = self.system_audio[language][message_key]
        
        pygame.mixer.music.load(audio_file)
        pygame.mixer.music.play()
        while pygame.mixer.music.get_busy():
            time.sleep(0.1)
    
    def scan_document(self):
        """Scan document using connected scanner"""
        logger.info("Initiating document scan")
        
        timestamp = int(time.time())
        output_path = f"{self.config['temp_path']}/scan_{timestamp}.png"
        
        try:
            # Configure scanner settings
            self.scanner.set_resolution(self.config["scan_resolution"])
            self.scanner.set_color_mode("color")
            self.scanner.set_document_size(self.config["max_scan_size"])
            
            # Perform scan
            scan_successful = self.scanner.scan(output_path)
            
            if not scan_successful:
                raise RuntimeError("Scanner failed to complete scan operation")
            
            logger.info(f"Document scanned successfully: {output_path}")
            return output_path
            
        except Exception as e:
            logger.error(f"Scan failed: {str(e)}")
            raise RuntimeError(f"Failed to scan document: {str(e)}")
    
    def analyze_document(self, document_path):
        """Analyze the scanned document"""
        logger.info(f"Analyzing document: {document_path}")
        
        try:
            # Determine document type
            doc_type = self._detect_document_type(document_path)
            logger.info(f"Detected document type: {doc_type}")
            
            # Process based on document type
            if doc_type == "xray":
                result = self._analyze_xray(document_path)
            elif doc_type == "mri":
                result = self._analyze_mri(document_path)
            elif doc_type == "ct":
                result = self._analyze_ct(document_path)
            elif doc_type == "ecg":
                result = self._analyze_ecg(document_path)
            elif doc_type == "text_report":
                result = self._analyze_text_report(document_path)
            else:
                # Use API for unknown document types
                result = self._analyze_via_api(document_path, doc_type)
            
            # Save analysis result
            timestamp = int(time.time())
            result_path = f"{self.config['output_path']}/result_{timestamp}.json"
            with open(result_path, "w") as f:
                json.dump(result, f, indent=2)
            
            logger.info(f"Analysis complete, results saved to {result_path}")
            return result
            
        except Exception as e:
            logger.error(f"Analysis failed: {str(e)}")
            raise RuntimeError(f"Failed to analyze document: {str(e)}")
    
    def _detect_document_type(self, document_path):
        """Detect the type of medical document"""
        image = cv2.imread(document_path)
        
        # Extract text for classification
        extracted_text = pytesseract.image_to_string(image)
        
        # Check for keywords to determine document type
        text_lower = extracted_text.lower()
        
        if "x-ray" in text_lower or "radiograph" in text_lower:
            return "xray"
        elif "mri" in text_lower or "magnetic resonance" in text_lower:
            return "mri"
        elif "ct scan" in text_lower or "computed tomography" in text_lower:
            return "ct"
        elif "
