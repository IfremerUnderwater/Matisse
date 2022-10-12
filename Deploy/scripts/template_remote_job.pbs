#!/bin/bash
#PBS -m abe
#PBS -M <%job.owner.email%>
#PBS -q <%scheduler.queue.name%>
#PBS -l walltime=20:00:00
#PBS -l mem=64000mb
#PBS -l ncpus=<%job.resources.ncpus%>
#PBS -o <%jobs.root.path%>/<%job.export.name%>.out
#PBS -e <%jobs.root.path%>/<%job.export.name%>.err

MATISSE_SCRIPT_PATH=<%launch.script.path%>
MATISSE_SCRIPT_BIND=<%launch.script.binding%>
MATISSE_BIN_PATH=<%bin.root.path%>
MATISSE_BIN_BIND=<%bin.root.binding%>

MATISSE_FILES_ROOT=<%runtime.files.root.path%>
MATISSE_DATASETS_DIR=<%runtime.datasets.dir%>
MATISSE_DATASETS_BIND=<%runtime.datasets.binding%>
MATISSE_JOBS_DIR=<%runtime.jobs.subdir%>
MATISSE_JOBS_BIND=<%runtime.jobs.binding%>
MATISSE_RESULTS_DIR=<%runtime.results.subdir%>
MATISSE_RESULTS_BIND=<%runtime.results.binding%>

MATISSE_IMG=<%server.container.image%>
MATISSE_CMD=~/$MATISSE_SCRIPT_BIND/<%launch.script.name%>

. /etc/profile.d/modules.sh
module load singularity/3.4.1
singularity exec -B $MATISSE_SCRIPT_PATH:$HOME/$MATISSE_SCRIPT_BIND -B $MATISSE_BIN_PATH:$HOME/$MATISSE_BIN_BIND -B $MATISSE_DATASETS_DIR:$HOME/$MATISSE_DATASETS_BIND -B $MATISSE_FILES_ROOT/$MATISSE_JOBS_DIR:$HOME/$MATISSE_JOBS_BIND -B $MATISSE_FILES_ROOT/$MATISSE_RESULTS_DIR:$HOME/$MATISSE_RESULTS_BIND $MATISSE_IMG $MATISSE_CMD <%jobs.root.path.bound%> <%job.name%>

