import pybgmix
import sys
from celery import Celery
from celery.utils.log import get_task_logger

logger = get_task_logger(__name__)
sys.stdout = logger.info

mixnet_app = Celery('mixnet',
                    backend='rpc://',
                    broker='pyamqp://guest@localhost//')

ciphers_file = "py_ciphers.txt"

@mixnet_app.task
def generate_ciphers():
    pybgmix.bg_generate_ciphers(ciphers_file)

@mixnet_app.task
def mix():
    pybgmix.bg_mix(ciphers_file)
