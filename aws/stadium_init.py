# launch spot fleet requests

import hashlib
import datetime
import boto3
import time

ec2 = boto3.resource('ec2')
client = boto3.client('ec2')

# BEGIN CONFIGURABLE

# general config
num_instances = 1
num_instances = 12
# num_instances = 110
instance_type = 'm3.medium'
instance_type = 'c4.8xlarge'
product_description = 'Linux/UNIX'
ami_id = 'ami-00000000'
security_group_id = 'sg-00000000'
key_name = 'derek-stadium' # note: take off the .pem extension here
iam_fleet_role = 'arn:aws:iam::000000000000:role/aws-ec2-spot-fleet-role'

# timing for launching and waiting for instances
num_request_retries = 3
instance_wait_ready_sleep_interval = 15

# for price history
start_time = datetime.datetime.now() - datetime.timedelta(days=1)
end_time = datetime.datetime.now()
price_med_window = 5
# bid_scale = 1.1
bid_scale = 4.0

# END CONFIGURABLE

# get spot price history

history = client.describe_spot_price_history(
    StartTime = start_time,
    EndTime = end_time,
    InstanceTypes = [instance_type],
    ProductDescriptions = [product_description]
)

# take median price
price = min(x['SpotPrice'] for x in history['SpotPriceHistory'][:price_med_window])
bid = float(price) * bid_scale

print 'launching instance type {}'.format(instance_type)
print 'price (min window {}): {}'.format(price_med_window, price)
print 'bid (price * {}): {}'.format(bid_scale, bid)
print 'total (price * {} instance(s)): {}'.format(num_instances, float(price) * num_instances)

ok = raw_input('ok ([y]/n)? ')
if ok == '':
    ok = 'y'

if ok == 'y':
    print 'placing bid at {} for {} instance(s) (total {})'.format(bid, num_instances, bid * num_instances)
else:
    print 'script halting'
    raise Exception

# construct launch request

h = hashlib.sha256()
h.update(str(datetime.datetime.now()))
idempotency_token = h.hexdigest()

request_config = {
    'ClientToken': idempotency_token,
    'SpotPrice': str(bid),
    'TargetCapacity': num_instances,
    'TerminateInstancesWithExpiration': True,
    'IamFleetRole': iam_fleet_role,
    'LaunchSpecifications': [{
        'ImageId': ami_id,
        'KeyName': key_name,
        'SecurityGroups': [{
            # 'GroupName': security_group_name,
            'GroupId': security_group_id,
        }],
        'InstanceType': instance_type,
    }],
    'AllocationStrategy': 'lowestPrice',
    'Type': 'request',
}

# launch request

for i in range(num_request_retries):
    print 'sending request... ({} of {})'.format(i+1, num_request_retries)
    response = client.request_spot_fleet(
        SpotFleetRequestConfig = request_config,
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
    print 'requested spot fleet!'
    print 'response:'
    print response

rid = response['SpotFleetRequestId']

# wait for instances to launch

# TODO should be fetching by request id and loop until ActivityStatus fulfilled

print 'waiting for instances to be ready...'

while True:
    time.sleep(instance_wait_ready_sleep_interval)

    print 'sending poll'
    response = client.describe_spot_fleet_instances(
        SpotFleetRequestId = rid
    )
    print 'received response'
    print response
    instances = response['ActiveInstances']
    print '{} of {} up'.format(len(instances), num_instances)
    if len(instances) != num_instances:
        continue

    iids = [x['InstanceId'] for x in instances]
    break

response = client.describe_instances(
    InstanceIds = iids
)
reservations = response['Reservations']
assert len(reservations) == 1
instances = response['Reservations'][0]['Instances']
assert len(instances) == num_instances

print 'instance IPs:'
ips = [x['PublicIpAddress'] for x in instances]
for ip in ips:
    print ip
