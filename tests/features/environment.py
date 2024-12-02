import logging
import os

def before_all(context):
    log_dir = "_output"
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)

    log_file = os.path.join(log_dir, "behave_execution.log")
    logger = logging.getLogger("WEBSERVER")
    logger.setLevel(logging.DEBUG)

    file_handler = logging.FileHandler(log_file, mode='a')
    file_handler.setLevel(logging.DEBUG)

    formatter = logging.Formatter(
        '[%(asctime)s][%(levelname)s]: %(message)s',
        datefmt='%Y-%m-%d %H:%M:%S'
    )
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    context.logger = logger
    logger.info("Logger start.")
