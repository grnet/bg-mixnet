import pybgmix
import sys
import json
from celery import Celery
from celery.utils.log import get_task_logger

logger = get_task_logger(__name__)

mixnet_app = Celery('mixnet',
                    backend='rpc://',
                    broker='pyamqp://guest@localhost//')

ciphers_file = "ciphers.json"

@mixnet_app.task
def generate_ciphers(dim_m, dim_n):
    try:
        from zeus_legacy.zeus.core import ZeusCoreElection

        # Does zeus fork() within mk_random()? Without the following
        # statement the Crypto module raises an AssertionError
        # suggesting that Random.atfork() is required.
        from Crypto import Random
        Random.atfork()

        election = ZeusCoreElection.mk_random(
                            nr_votes        =   dim_m * dim_n,
                            stage           =   "VOTING",
                            )
        mix, counted_list = election.extract_votes_for_mixing()
	with open(ciphers_file, 'w') as f:
		json.dump(mix, f)
        # print "mix: %s" % str(mix)
        # print "counted_list: %s" % str(counted_list)
    except ImportError:
        pybgmix.bg_generate_ciphers(ciphers_file, dim_m, dim_n)

@mixnet_app.task
def mix(dim_m, dim_n):
    pybgmix.bg_mix(ciphers_file, dim_m, dim_n)
