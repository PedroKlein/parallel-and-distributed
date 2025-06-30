FROM ubuntu:22.04

# Install OpenMPI, SSH, Python3, Slurm (optional)
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    openssh-server openmpi-bin openmpi-common libopenmpi-dev python3 python3-pip sudo iputils-ping make \
    slurm-wlm && \
    rm -rf /var/lib/apt/lists/*

# Set up SSH
RUN mkdir /var/run/sshd && \
    echo 'root:root' | chpasswd && \
    sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
    sed -i 's@session\s*required\s*pam_loginuid.so@session optional pam_loginuid.so@g' /etc/pam.d/sshd

# Allow passwordless ssh
RUN ssh-keygen -A && \
    mkdir -p /root/.ssh && \
    ssh-keygen -t rsa -f /root/.ssh/id_rsa -q -N "" && \
    cat /root/.ssh/id_rsa.pub >> /root/.ssh/authorized_keys && \
    chmod 600 /root/.ssh/authorized_keys

# Ensure SLURM state directories exist
RUN mkdir -p /var/spool/slurmctld /var/spool/slurmd

# Set environment
ENV NOTVISIBLE="in users profile"
ENV PATH="/usr/lib64/openmpi/bin:$PATH"
# ENV PATH="/usr/lib/x86_64-linux-gnu/openmpi/bin:$PATH"

# Expose SSH
EXPOSE 22

COPY slurm.conf /etc/slurm/slurm.conf
COPY scripts/entrypoint.sh /usr/local/bin/entrypoint.sh
RUN chmod +x /usr/local/bin/entrypoint.sh

CMD ["/usr/sbin/sshd", "-D"]
