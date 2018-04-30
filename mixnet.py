import pybgmix
from celery import Celery
from celery.utils.log import get_task_logger

logger = get_task_logger(__name__)

mixnet_app = Celery('mixnet',
                    backend='rpc://',
                    broker='pyamqp://guest@localhost//')


@mixnet_app.task
def encrypt_cipher(i, modulus, generator, order, public_key):
    from zeus_legacy.zeus.core import encrypt
    if i % 1000 == 0:
        logger.info("Ciphers encrypted: %d", i)
    return encrypt(i, modulus, generator, order, public_key)[0:2]

@mixnet_app.task
def generate_ciphers(ciphers_file, dim_m, dim_n):
    return pybgmix.bg_generate_ciphers(ciphers_file, dim_m, dim_n)

@mixnet_app.task
def mix(ciphers_file, dim_m, dim_n):
    return pybgmix.bg_mix(ciphers_file, dim_m, dim_n)
