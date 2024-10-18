import logging
from logging.handlers import RotatingFileHandler
from datetime import datetime

class LogUtility:
    def __init__(self, log_file='app.log', max_bytes=10485760, backup_count=5):
        self.logger = logging.getLogger('LogUtility')
        self.logger.setLevel(logging.DEBUG)
        
        handler = RotatingFileHandler(log_file, maxBytes=max_bytes, backupCount=backup_count)
        handler.setLevel(logging.DEBUG)
        
        formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
        handler.setFormatter(formatter)
        
        self.logger.addHandler(handler)
        
    def log_debug(self, message):
        self.logger.debug(message)
        
    def log_info(self, message):
        self.logger.info(message)
        
    def log_warning(self, message):
        self.logger.warning(message)
        
    def log_error(self, message):
        self.logger.error(message)
        
    def log_critical(self, message):
        self.logger.critical(message)

# Example usage
if __name__ == '__main__':
    log_utility = LogUtility('my_app.log')
    log_utility.log_info('This is an info message.')
    log_utility.log_debug('This is a debug message.')
    log_utility.log_warning('This is a warning message.')
    log_utility.log_error('This is an error message.')
    log_utility.log_critical('This is a critical message.')
