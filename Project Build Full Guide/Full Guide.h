# Setup Instructions for AI-Driven Medical Imaging Disease Detection System
**Created by: Ashwanth**

This guide provides detailed instructions for setting up both the hardware and software components of the AI-Driven Medical Imaging Disease Detection system.

## Table of Contents
1. [Hardware Setup](#hardware-setup)
   - [Components Required](#components-required)
   - [Hardware Assembly](#hardware-assembly)
   - [Scanner Modification](#scanner-modification)
   - [Orange Pi Configuration](#orange-pi-configuration)
   - [Audio Output Setup](#audio-output-setup)
   - [Button Interface](#button-interface)
   - [Power Supply](#power-supply)
   - [Enclosure Assembly](#enclosure-assembly)

2. [Software Setup](#software-setup)
   - [Operating System Installation](#operating-system-installation)
   - [Dependencies Installation](#dependencies-installation)
   - [AI Models Setup](#ai-models-setup)
   - [API Configuration](#api-configuration)
   - [Language Support Setup](#language-support-setup)
   - [System Configuration](#system-configuration)
   - [Autostart Configuration](#autostart-configuration)

3. [Testing and Calibration](#testing-and-calibration)
   - [Scanner Calibration](#scanner-calibration)
   - [System Testing](#system-testing)
   - [Performance Optimization](#performance-optimization)

4. [Troubleshooting](#troubleshooting)
   - [Common Issues](#common-issues)
   - [Diagnostic Tools](#diagnostic-tools)
   - [Support Contact](#support-contact)

---

## Hardware Setup

### Components Required

1. **Computing Unit**:
   - Orange Pi 5 Plus (8GB RAM model recommended)
   - 64GB+ microSD card (Class 10 or higher)
   - 5V/4A Power Supply with USB-C connector

2. **Scanner Components**:
   - Canon LiDE 300 document scanner (or equivalent)
   - USB cable (Type-A to Type-B)
   - Scanner modification components (see Scanner Modification section)

3. **Audio Components**:
   - USB speaker or 3.5mm audio speaker
   - PAM8403 audio amplifier module (if using passive speakers)
   - Speaker wire

4. **User Interface**:
   - Momentary push button (16mm diameter, illuminated)
   - 3-position toggle switch for language selection
   - 16x2 LCD display module (I2C interface)
   - LED indicators (3 colors: green, red, blue)

5. **Enclosure and Mounting**:
   - Custom 3D printed enclosure (STL files provided)
   - M3 screws and nuts (assorted lengths)
   - Rubber feet
   - Cable management components

6. **Miscellaneous**:
   - Jumper wires
   - Breadboard or prototype PCB
   - Heat sinks for Orange Pi
   - Cooling fan (40mm)
   - Wire connectors and crimping tools

### Hardware Assembly

#### Step 1: Prepare the Orange Pi
1. Attach heat sinks to the CPU, GPU, and RAM chips on the Orange Pi.
2. Mount the Orange Pi to the base plate of the enclosure using M3 screws.
3. Connect the cooling fan to the 5V and GND pins on the Orange Pi GPIO.

#### Step 2: Scanner Modification
The document scanner needs to be modified to interface directly with our system:

1. Carefully disassemble the scanner's outer casing.
2. Locate the scanner's control board.
3. Solder jumper wires to the scanner's button interface points.
4. Route these wires through a small hole in the scanner casing.
5. Reassemble the scanner, ensuring no cables are pinched.

#### Step 3: Button Interface Wiring
1. Mount the main scan button on the front panel of the enclosure.
2. Connect the button to GPIO pins on the Orange Pi:
   - Button positive terminal → GPIO17
   - Button negative terminal → GND
3. For the illuminated part of the button:
   - Positive terminal → GPIO27 (through a 220Ω resistor)
   - Negative terminal → GND

#### Step 4: Language Selection Switch
1. Mount the 3-position toggle switch on the front panel.
2. Connect the switch to GPIO pins:
   - Common terminal → GND
   - Position 1 (English) → GPIO22
   - Position 2 (Tamil) → GPIO23
   - Position 3 (Malayalam) → GPIO24

#### Step 5: LCD Display
1. Mount the 16x2 LCD display on the front panel.
2. Connect the I2C interface:
   - VCC → 5V
   - GND → GND
   - SDA → GPIO2 (I2C1 SDA)
   - SCL → GPIO3 (I2C1 SCL)

#### Step 6: Status LEDs
1. Mount the three status LEDs on the front panel.
2. Connect each LED (through 220Ω resistors):
   - Green (Ready) → GPIO5 → GND
   - Red (Error) → GPIO6 → GND
   - Blue (Processing) → GPIO13 → GND

#### Step 7: Audio Output
1. If using the 3.5mm audio jack:
   - Connect directly to the Orange Pi's audio output jack
2. If using USB speakers:
   - Connect to any available USB port
3. If using the PAM8403 amplifier:
   - Connect amplifier input to Orange Pi audio output
   - Connect amplifier output to speakers
   - Connect amplifier power to 5V and GND

#### Step 8: Scanner Connection
1. Connect the modified scanner to a USB port on the Orange Pi.
2. Connect the scanner control wires to GPIO pins:
   - Scanner button signal → GPIO26
   - Scanner status LED → GPIO16
   - Scanner GND → GND

#### Step 9: Final Assembly
1. Carefully route all cables inside the enclosure.
2. Use cable ties and adhesive cable holders for cable management.
3. Secure all components to prevent movement.
4. Attach the enclosure top cover using M3 screws.
5. Apply rubber feet to the bottom of the enclosure.

### Power Supply
1. Connect the 5V/4A power supply to the Orange Pi's USB-C port.
2. If additional power is needed for peripherals, use a powered USB hub.

---

## Software Setup

### Operating System Installation

1. Download the latest Armbian OS image for Orange Pi 5 Plus from the official website.
2. Flash the image to the microSD card using Balena Etcher or similar software.
3. Insert the microSD card into the Orange Pi and power it on.
4. Complete the initial setup process:
   ```bash
   # Set root password when prompted
   # Create a new user when prompted
   sudo apt update
   sudo apt upgrade -y
   ```

### Dependencies Installation

1. Install system dependencies:
   ```bash
   sudo apt install -y python3-pip python3-dev python3-numpy python3-opencv
   sudo apt install -y libatlas-base-dev libjasper-dev libqt5gui5
   sudo apt install -y libgstreamer1.0-0 libgstreamer-plugins-base1.0-0
   sudo apt install -y libavcodec-dev libavformat-dev libswscale-dev
   sudo apt install -y libgtk-3-dev libcanberra-gtk3-module
   sudo apt install -y libtiff5-dev libjpeg-dev libpng-dev
   sudo apt install -y tesseract-ocr tesseract-ocr-eng tesseract-ocr-tam tesseract-ocr-mal
   sudo apt install -y python3-pygame python3-pil python3-smbus i2c-tools
   sudo apt install -y sane-utils libsane-dev
   ```

2. Install Python dependencies:
   ```bash
   pip3 install --upgrade pip
   pip3 install numpy tensorflow torch torchvision torchaudio
   pip3 install opencv-python pillow pytesseract
   pip3 install transformers datasets
   pip3 install gTTS pygame
   pip3 install RPi.GPIO OPi.GPIO
   pip3 install requests pydub
   pip3 install scikit-learn scikit-image
   pip3 install flask gunicorn
   ```

3. Configure scanner access permissions:
   ```bash
   sudo usermod -a -G scanner,dialout $USER
   sudo cp /etc/sane.d/dll.conf /etc/sane.d/dll.conf.backup
   echo "canon_lide" | sudo tee -a /etc/sane.d/dll.conf
   ```

### AI Models Setup

1. Create directories for the project:
   ```bash
   mkdir -p ~/medical_imaging_ai/{models,temp,results,logs}
   ```

2. Download pre-trained models:
   ```bash
   cd ~/medical_imaging_ai/models
   
   # Download X-ray model
   wget https://github.com/ashwanth/medical-imaging-ai/releases/download/v1.0/xray_model.pth
   
   # Download MRI model
   wget https://github.com/ashwanth/medical-imaging-ai/releases/download/v1.0/mri_model.pth
   
   # Download CT model
   wget https://github.com/ashwanth/medical-imaging-ai/releases/download/v1.0/ct_model.h5
   
   # Download ECG model
   wget https://github.com/ashwanth/medical-imaging-ai/releases/download/v1.0/ecg_model.h5
   
   # Download medical terminology database
   wget https://github.com/ashwanth/medical-imaging-ai/releases/download/v1.0/medical_terminology.json
   ```

3. Clone the project repository:
   ```bash
   cd ~
   git clone https://github.com/ashwanth/medical-imaging-ai.git
   cd medical-imaging-ai
   ```

### API Configuration

1. Create API configuration file:
   ```bash
   cd ~/medical-imaging-ai
   cp config.example.json config.json
   ```

2. Edit the configuration file:
   ```bash
   nano config.json
   ```
   
   Update the following fields:
   - `api_key`: Your API key for the medical imaging service
   - `scanner_device`: Path to your scanner device (usually `/dev/usb/scanner0`)
   - `use_local_models`: Set to `true` for offline operation
   - `use_api_fallback`: Set to `true` to use API when local models fail

### Language Support Setup

1. Configure text-to-speech for multiple languages:
   ```bash
   # Test TTS for each language
   python3 -c "from gtts import gTTS; tts = gTTS('This is a test', lang='en'); tts.save('test_en.mp3')"
   python3 -c "from gtts import gTTS; tts = gTTS('இது ஒரு சோதனை', lang='ta'); tts.save('test_ta.mp3')"
   python3 -c "from gtts import gTTS; tts = gTTS('ഇത് ഒരു പരീക്ഷണമാണ്', lang='ml'); tts.save('test_ml.mp3')"
   
   # Play test files
   pygame -c "import pygame; pygame.mixer.init(); pygame.mixer.music.load('test_en.mp3'); pygame.mixer.music.play(); import time; time.sleep(3)"
   ```

2. Download language models for OCR:
   ```bash
   sudo apt install -y tesseract-ocr-all
   ```

### System Configuration

1. Create the system service file:
   ```bash
   sudo nano /etc/systemd/system/medical-imaging.service
   ```
   
   Add the following content:
   ```
   [Unit]
   Description=AI-Driven Medical Imaging Disease Detection System
   After=network.target
   
   [Service]
   User=<your-username>
   WorkingDirectory=/home/<your-username>/medical-imaging-ai
   ExecStart=/usr/bin/python3 /home/<your-username>/medical-imaging-ai/main.py
   Restart=always
   RestartSec=10
   
   [Install]
   WantedBy=multi-user.target
   ```

2. Enable I2C interface for LCD display:
   ```bash
   sudo armbian-config
   # Navigate to System > Hardware
   # Enable I2C
   # Save and exit
   ```

3. Enable the service:
   ```bash
   sudo systemctl enable medical-imaging.service
   sudo systemctl start medical-imaging.service
   ```

### Autostart Configuration

1. Create a desktop shortcut (if using desktop environment):
   ```bash
   mkdir -p ~/.config/autostart
   nano ~/.config/autostart/medical-imaging.desktop
   ```
   
   Add the following content:
   ```
   [Desktop Entry]
   Type=Application
   Name=Medical Imaging System
   Comment=AI-Driven Medical Imaging Disease Detection System
   Exec=/usr/bin/python3 /home/<your-username>/medical-imaging-ai/main.py
   Terminal=false
   X-GNOME-Autostart-enabled=true
   ```

2. Configure system to start on boot in headless mode:
   ```bash
   sudo systemctl enable medical-imaging.service
   ```

---

## Testing and Calibration

### Scanner Calibration

1. Run the scanner calibration utility:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/calibrate_scanner.py
   ```

2. Follow the on-screen instructions to:
   - Adjust brightness and contrast
   - Set the scan area dimensions
   - Calibrate color accuracy
   - Test scan speed and resolution

### System Testing

1. Test the complete system:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/system_test.py
   ```

2. Verify each component:
   - Scanner functionality
   - Button interface
   - Language selection
   - AI model loading
   - Text-to-speech output
   - LCD display

3. Run a complete test with sample medical images:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/run_sample_tests.py
   ```

### Performance Optimization

1. Adjust system configuration for better performance:
   ```bash
   cd ~/medical-imaging-ai
   nano config.json
   ```
   
   Optimize these settings:
   - `scan_resolution`: Lower for faster scanning (e.g., 200 DPI)
   - `use_gpu`: Set to `true` if GPU acceleration is available
   - `batch_size`: Adjust based on available memory
   - `audio_quality`: Adjust for balance between quality and speed

2. Monitor system performance:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/performance_monitor.py
   ```

---

## Troubleshooting

### Common Issues

1. **Scanner not detected**
   - Check USB connection
   - Verify scanner is powered on
   - Run `sudo sane-find-scanner` to detect connected scanners
   - Check permissions with `ls -l /dev/usb/`

2. **Audio not working**
   - Check speaker connections
   - Run `aplay -l` to list audio devices
   - Test with `speaker-test -c2 -t wav`
   - Verify volume settings with `alsamixer`

3. **Button not responding**
   - Check wiring connections
   - Test GPIO pin with `gpio -g read 17`
   - Verify permissions for GPIO access

4. **AI models failing to load**
   - Check disk space with `df -h`
   - Verify model files exist in the models directory
   - Check for Python errors in the logs

5. **System crashes during analysis**
   - Check memory usage with `free -h`
   - Monitor CPU temperature with `vcgencmd measure_temp`

   - Reduce resolution or use API fallback for large images
   - Check logs for specific error messages

### Diagnostic Tools

1. **System Diagnostics**
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/diagnostics.py
   ```
   This will run a comprehensive system check and generate a diagnostic report.

2. **Hardware Test**
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/hardware_test.py
   ```
   Tests all hardware components individually and reports any issues.

3. **Log Analysis**
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/log_analyzer.py
   ```
   Analyzes system logs to identify patterns and potential issues.

4. **Network Connectivity Test**
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/network_test.py
   ```
   Tests API connectivity and network performance.

5. **Model Verification**
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/verify_models.py
   ```
   Verifies all AI models are correctly loaded and functioning.

### Support Contact

For technical support or questions about this system, please contact:

**Ashwanth Kumar**  
Email: ashwanth.kumar@medicalimaging.ai  
GitHub: https://github.com/ashwanth/medical-imaging-ai  
Support Ticket System: https://support.medicalimaging.ai

When reporting issues, please include:
- System diagnostic report
- Relevant log files
- Description of the problem
- Steps to reproduce the issue
- Any error messages displayed

---

## Advanced Configuration

### Custom Model Integration

You can integrate your own custom-trained models for specific medical imaging tasks:

1. Prepare your model in TensorFlow, PyTorch, or ONNX format
2. Place the model file in the `~/medical-imaging-ai/models/custom/` directory
3. Create a configuration file for your model:
   ```bash
   nano ~/medical-imaging-ai/models/custom/my_model_config.json
   ```
   
   Example configuration:
   ```json
   {
     "model_name": "custom_lung_nodule_detector",
     "model_file": "lung_nodule_v2.pth",
     "model_type": "pytorch",
     "input_size": [224, 224],
     "preprocessing": "normalize",
     "class_labels": ["normal", "nodule_benign", "nodule_malignant"],
     "confidence_threshold": 0.75
   }
   ```

4. Register your model in the main configuration:
   ```bash
   nano ~/medical-imaging-ai/config.json
   ```
   
   Add to the `custom_models` section:
   ```json
   "custom_models": [
     {
       "name": "custom_lung_nodule_detector",
       "config_file": "models/custom/my_model_config.json",
       "document_types": ["xray_chest"]
     }
   ]
   ```

### Multi-language Support Extension

To add support for additional languages:

1. Install required language packages:
   ```bash
   sudo apt install -y tesseract-ocr-<lang-code>
   ```

2. Add the language to your configuration:
   ```bash
   nano ~/medical-imaging-ai/config.json
   ```
   
   Update the supported languages:
   ```json
   "supported_languages": ["english", "tamil", "malayalam", "<new-language>"]
   ```

3. Add translation model for the new language:
   ```bash
   cd ~/medical-imaging-ai/models
   wget https://github.com/Helsinki-NLP/Tatoeba-Challenge/raw/master/models/eng-<lang>/opus-2020-07-03.zip
   unzip opus-2020-07-03.zip
   ```

4. Test the new language:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/test_language.py --lang <new-language>
   ```

### Remote Monitoring Setup

To enable remote monitoring of the system:

1. Install monitoring server:
   ```bash
   sudo apt install -y nginx
   pip3 install flask-socketio
   ```

2. Configure the monitoring server:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/setup_monitoring.py
   ```

3. Enable the monitoring service:
   ```bash
   sudo systemctl enable medical-imaging-monitor.service
   sudo systemctl start medical-imaging-monitor.service
   ```

4. Access the monitoring dashboard at:
   ```
   http://<orange-pi-ip>:8080
   ```

### Backup and Recovery

1. Create a backup script:
   ```bash
   nano ~/medical-imaging-ai/tools/backup.sh
   ```
   
   Add the following content:
   ```bash
   #!/bin/bash
   BACKUP_DIR="/media/backup/medical-imaging-$(date +%Y%m%d)"
   mkdir -p $BACKUP_DIR
   
   # Backup configuration
   cp ~/medical-imaging-ai/config.json $BACKUP_DIR/
   
   # Backup models
   tar -czf $BACKUP_DIR/models.tar.gz ~/medical-imaging-ai/models/
   
   # Backup logs
   tar -czf $BACKUP_DIR/logs.tar.gz ~/medical-imaging-ai/logs/
   
   # Backup results
   tar -czf $BACKUP_DIR/results.tar.gz ~/medical-imaging-ai/results/
   
   # Backup system configuration
   sudo cp /etc/systemd/system/medical-imaging.service $BACKUP_DIR/
   
   echo "Backup completed to $BACKUP_DIR"
   ```

2. Make the script executable:
   ```bash
   chmod +x ~/medical-imaging-ai/tools/backup.sh
   ```

3. Schedule regular backups:
   ```bash
   crontab -e
   ```
   
   Add the following line for weekly backups:
   ```
   0 2 * * 0 ~/medical-imaging-ai/tools/backup.sh
   ```

4. To restore from backup:
   ```bash
   cd ~/medical-imaging-ai/tools
   ./restore.sh /path/to/backup/directory
   ```

## Security Considerations

### Data Privacy

1. Enable data encryption:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/setup_encryption.py
   ```
   This will:
   - Generate encryption keys
   - Configure encrypted storage for patient data
   - Set up secure deletion policies

2. Configure data retention policy:
   ```bash
   nano ~/medical-imaging-ai/config.json
   ```
   
   Update the data retention settings:
   ```json
   "data_retention": {
     "scan_images": 30,  // days
     "analysis_results": 90,  // days
     "system_logs": 180  // days
   }
   ```

3. Set up secure user authentication:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/setup_authentication.py
   ```

### Network Security

1. Configure firewall:
   ```bash
   sudo apt install -y ufw
   sudo ufw default deny incoming
   sudo ufw default allow outgoing
   sudo ufw allow ssh
   sudo ufw allow 8080/tcp  # For monitoring dashboard
   sudo ufw enable
   ```

2. Set up secure API communication:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/setup_api_security.py
   ```
   This will configure:
   - HTTPS for API communication
   - API key rotation
   - Request rate limiting

## Performance Tuning

### Hardware Acceleration

1. Enable GPU acceleration (if available):
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/setup_gpu.py
   ```

2. Optimize memory usage:
   ```bash
   sudo nano /etc/sysctl.conf
   ```
   
   Add the following lines:
   ```
   vm.swappiness=10
   vm.vfs_cache_pressure=50
   ```

3. Apply changes:
   ```bash
   sudo sysctl -p
   ```

### Software Optimization

1. Profile the application:
   ```bash
   cd ~/medical-imaging-ai
   python3 -m cProfile -o profile.out main.py --test-mode
   python3 tools/analyze_profile.py profile.out
   ```

2. Optimize based on profiling results:
   ```bash
   cd ~/medical-imaging-ai
   python3 tools/optimize.py
   ```

## Final Notes

This AI-Driven Medical Imaging Disease Detection system is designed to assist healthcare professionals in analyzing medical images. It is not intended to replace professional medical diagnosis. Always consult with qualified healthcare providers for medical advice and treatment.

The system is developed by Ashwanth Kumar as a tool to improve accessibility to medical image analysis, particularly in areas with limited access to specialized radiologists.

For the latest updates and improvements, please check the GitHub repository regularly.

---

© 2025 Ashwanth . All rights reserved.

