import pybgmix
import sys
from celery import Celery
from celery.utils.log import get_task_logger
from Crypto import Random

logger = get_task_logger(__name__)

mixnet_app = Celery('mixnet',
                    backend='rpc://',
                    broker='pyamqp://guest@localhost//')

ciphers_file = "py_ciphers.txt"

@mixnet_app.task
def generate_ciphers():
    try:
        from zeus_legacy.zeus.core import ZeusCoreElection
        # Does zeus fork() within mk_random()? Without the following
        # statement the Crypto module raises an AssertionError
        # suggesting that Random.atfork() is required.
        Random.atfork()

        election = ZeusCoreElection.mk_random(
                            nr_votes        =   10000,
                            stage           =   "VOTING",
                            )
        mix, counted_list = election.extract_votes_for_mixing()
        print "mix: %s" % str(mix)
        print "counted_list: %s" % str(counted_list)
    except ImportError:
        pybgmix.bg_generate_ciphers(ciphers_file)

@mixnet_app.task
def mix():
    pybgmix.bg_mix(ciphers_file)
