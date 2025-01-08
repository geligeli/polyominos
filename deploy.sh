#!/bin/bash

set -ex

VM=c3d-180
ZONE=us-central1-f

gcloud compute ssh geli@$VM --command "killall puzzle_maker; rm -f /home/geli/puzzle_maker" --zone=$ZONE
gcloud compute scp ./puzzle_maker geli@$VM:/home/geli --zone=$ZONE


# gcloud compute instances create c3d-180 \
#     --project=aiproject-411418 \
#     --zone=us-central1-f \
#     --machine-type=c3d-highcpu-180 \
#     --network-interface=network-tier=PREMIUM,nic-type=GVNIC,stack-type=IPV4_ONLY,subnet=default \
#     --maintenance-policy=MIGRATE \
#     --provisioning-model=STANDARD \
#     --service-account=271857693621-compute@developer.gserviceaccount.com \
#     --scopes=https://www.googleapis.com/auth/devstorage.read_only,https://www.googleapis.com/auth/logging.write,https://www.googleapis.com/auth/monitoring.write,https://www.googleapis.com/auth/service.management.readonly,https://www.googleapis.com/auth/servicecontrol,https://www.googleapis.com/auth/trace.append \
#     --create-disk=auto-delete=yes,boot=yes,device-name=c3d-180,image=projects/debian-cloud/global/images/debian-12-bookworm-v20241210,mode=rw,size=10,type=pd-balanced \
#     --no-shielded-secure-boot \
#     --shielded-vtpm \
#     --shielded-integrity-monitoring \
#     --labels=goog-ec-src=vm_add-gcloud \
#     --reservation-affinity=any