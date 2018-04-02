import pybgmix
import sys
from celery import Celery
from celery.utils.log import get_task_logger

logger = get_task_logger(__name__)
sys.stdout = logger.info

mixnet_app = Celery('mixnet', backend='rpc://', broker='pyamqp://guest@localhost//')

@mixnet_app.task
def mix():
    print "this is a print"
    logger.info("Start mix")
    pybgmix.bg_mix()
