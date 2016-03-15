import boto3

client = boto3.client('ec2')

# BEGIN CONFIGURABLE

num_request_retries = 3

# END CONFIGURABLE

outstanding_requests = client.describe_spot_fleet_requests()['SpotFleetRequestConfigs']

active = [x for x in outstanding_requests if x['SpotFleetRequestState'] == 'active']
assert len(active) == 1
rid = active[0]['SpotFleetRequestId']

print 'shut down spot fleet with request id {}'.format(rid)
ok = raw_input('ok (y/[n])? ')
if ok == '':
    ok == 'n'

if ok != 'y':
    print 'script halting'
    raise Exception

for i in range(num_request_retries):
    print 'sending request... ({} of {})'.format(i+1, num_request_retries)
    response = client.cancel_spot_fleet_requests(
        SpotFleetRequestIds = [rid],
        TerminateInstances = True,
    )
    ok = response['ResponseMetadata']['HTTPStatusCode']
    if ok == 200:
        break

if ok != 200:
    print 'request failed...?'
    print 'last response:'
    print response
    raise Exception
else:
    print 'cancelled spot fleet!'
    print 'response:'
    print response

