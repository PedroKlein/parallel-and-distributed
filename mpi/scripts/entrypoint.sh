#!/bin/bash
set -e

# Ensure log files exist and have correct permissions
touch /var/log/slurmctld.log /var/log/slurmd.log
chown root:root /var/log/slurmctld.log /var/log/slurmd.log

# Start SSH in the background
service ssh start

# Set up SLURM
if [ "$NODE_TYPE" = "master" ]; then
    echo "Clearing previous state and starting slurmctld on master..."
    # The -c flag clears previous state, crucial for ephemeral Docker environments
    slurmctld -c &
    # Give slurmctld a moment to initialize before workers try to connect
    sleep 2 
else
    echo "Waiting for master's slurmctld to be ready..."
    # Simple but effective way to wait for the master
    sleep 10 
fi

# Start the slurmd daemon on all nodes (master and workers)
echo "Starting slurmd on $(hostname)..."
slurmd &

echo "Container is up. Tailing logs to keep it running."
# Tailing the actual logs is more useful for debugging than /dev/null
tail -f /var/log/slurmctld.log /var/log/slurmd.log