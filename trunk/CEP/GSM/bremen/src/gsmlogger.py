#!/usr/bin/python
"""
Tools for logging in GSM package.
"""
import logging

LOGGERS = {}

def get_gsm_logger(log_name, file_name, use_console=False):
    """
    Get the two-way logger.
    """
    if log_name in LOGGERS:
        return LOGGERS[log_name]
    logger = logging.getLogger(log_name)
    formatter = logging.Formatter(
                    '%(asctime)-6s: %(name)s - %(levelname)s - %(message)s')
    file_handler = logging.FileHandler(filename=file_name)
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    if use_console:
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(formatter)
        logger.addHandler(console_handler)
    LOGGERS[log_name] = logger
    return logger
