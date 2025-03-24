# Contributing to ESP32/ESP32-S3 LoRa Project

Thanks for your interest in contributing to this project! This document provides simple guidelines to make the contribution process easy and effective.

## Getting Started

1. **Set up your environment**
   - Follow the installation instructions in the README.md
   - Make sure you have all required libraries installed

2. **Familiarize yourself with the code structure**
   - The project is organized in a modular way with separate files for different functionalities
   - Check the main components: LoRa communications, WiFi management, display interface, and system monitoring

## How to Contribute

### Reporting Bugs

If you find a bug, please create an issue with:
- A clear title and description of the bug
- Steps to reproduce the issue
- Expected behavior vs. actual behavior
- Hardware setup details (ESP32 or ESP32-S3, LoRa module, etc.)
- Any relevant logs or screenshots

### Suggesting Enhancements

Have an idea to improve the project? Create an issue with:
- A clear description of your proposed enhancement
- The benefits it would bring to the project
- Any implementation considerations

### Pull Requests

1. **Fork the repository**
2. **Create a branch** for your feature or bugfix
3. **Make your changes**, following the coding style of the project
4. **Test thoroughly** on your hardware
5. **Submit a pull request** with a clear description of the changes
   - Reference any related issues
   - Explain your implementation approach

### Coding Guidelines

- Keep code modular and maintainable
- Add comments for complex functionality
- Follow naming conventions used in the existing code
- Minimize memory usage when possible (this is important for embedded systems)
- Consider both ESP32 and ESP32-S3 compatibility

## Development Workflow

1. Choose an issue to work on or create a new one
2. Discuss your approach in the issue if significant changes are planned
3. Implement your changes
4. Test on actual hardware (both platforms if possible)
5. Submit your pull request

## Adding Support for New Hardware

When adding support for new hardware:
1. Create a dedicated branch
2. Update pin definitions in config.h
3. Document the hardware connections in README.md
4. Test thoroughly before submitting a PR

## Questions?

If you have questions about contributing, feel free to open an issue with your question.

Thanks for contributing to make this project better!
