#!/usr/bin/python
"""
Tools for logging in GSM package.
"""
import logging
import os
import platform

LOGGERS = {}

if platform.node() == 'ldb001':
    BASE_LOG_DIR = os.path.join(os.getenv('HOME'), 'GSM')
else:
    BASE_LOG_DIR = os.path.join(os.getenv('HOME'), 'prog', 'GSM')

USE_CONSOLE = False


def get_gsm_logger(log_name, file_name, use_console=USE_CONSOLE):
    """
    Get the two-way logger.
    """
    if log_name in LOGGERS:
        return LOGGERS[log_name]
    logger = logging.getLogger(log_name)
    logger.setLevel(logging.INFO)
    formatter = logging.Formatter(
                    '%(asctime)-6s: %(name)s - %(levelname)s - %(message)s')
    file_handler = logging.FileHandler(filename=os.path.join(BASE_LOG_DIR,
                                                          file_name))
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    if use_console:
        console_handler = logging.StreamHandler()
        console_handler.setFormatter(formatter)
        logger.addHandler(console_handler)
    LOGGERS[log_name] = logger
    USE_CONSOLE = use_console
    return logger


def switch_console(use_console=False):
    """
    Switch console output on/off for all loggers.
    """
    USE_CONSOLE = use_console
    for logger in LOGGERS.itervalues():
        if use_console and len(logger.handlers) == 1:
            console_handler = logging.StreamHandler()
            console_handler.setFormatter(logging.Formatter(
                    '%(asctime)-6s: %(name)s - %(levelname)s - %(message)s'))
            logger.addHandler(console_handler)
        elif not use_console and len(logger.handlers) == 2:
            logger.removeHandler(logger.handlers[1])


def set_all_levels(level):
    """
    Set output level for all loggers.
    """
    for logger in LOGGERS.itervalues():
        logger.setLevel(level)
