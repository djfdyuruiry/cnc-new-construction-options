# CNC Launcher Configuration Service

## Overview

This project implements a configuration service that manages launcher settings in a user's application data directory.

## Key Features

- Loads configuration from app data directory (`~/.cnc-launcher`) or default config.yml
- Saves configuration back to user app data directory
- Provides a shared reference to configuration
- Handles loading and saving of YAML configuration

## Implementation Details

### LauncherConfigService

The `LauncherConfigService` provides:

- `Load()` - Loads configuration from app data directory or default config
- `Save()` - Saves configuration to app data directory
- `Get()` - Gets current configuration reference
- `Set()` - Sets new configuration reference

### Usage

The service is designed to work with the existing dependency injection system in the application. It handles loading configuration from either the application data directory (for user-specific settings) or the default location (for application defaults).

### Configuration Storage Location

Configuration is stored in:
- User app data directory: `~/.cnc-launcher/launcher-config.yml`
- Fallback: `config.yml` in application directory

## Integration

This service replaces the old `LauncherConfigLoader` approach and provides a cleaner, more maintainable pattern for configuration management.

## Design Pattern

This follows a service pattern where:
1. Configuration is loaded once at startup
2. Configuration is shared as a singleton reference
3. Configuration can be saved back to persistent storage
4. Configuration loading and saving logic is encapsulated in the service
