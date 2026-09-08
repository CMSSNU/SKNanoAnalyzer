import os
SINGULARITY_IMAGE = os.environ['SINGULARITY_IMAGE']

main_job = {
    'universe': "vanilla",
    'executable': "run.sh",
    'JobBatchName': "batchname", 
    '+singularityimage': f'"{SINGULARITY_IMAGE}"',
    'RequestMemory': f'2000 MB', # 2 times of memory usage
    'RequestCpus': 1,
    'arguments': "$(Process)",
    'output': "job_$(Process).out",
    'error': "job_$(Process).err",
    'should_transfer_files': "YES",
    'when_to_transfer_output': "ON_EXIT_OR_EVICT",
    'concurrency_limits': "300",
    # periodic release for 3 times // Held reason is lack of memory // JobStatus is Hold // https://research.cs.wisc.edu/htcondor/manual/v8.5/12_Appendix_A.html
    'periodic_release': '(NumJobStarts < 5) && (HoldReasonCode == 34 || HoldReasonCode == 21) && (JobStatus == 5)' 
}

hadd_job = {
    'universe': "vanilla",
    'executable': "hadd.sh",
    'JobBatchName': "batchname",
    '+singularityimage': f'"{SINGULARITY_IMAGE}"',
    'RequestCpus': 8,
    'RequestMemory': '16384 MB',
    'output': "hadd.out",
    'error': "hadd.err",
    'should_transfer_files': "YES",
    'when_to_transfer_output': "ON_EXIT_OR_EVICT",
    'periodic_release': '(NumJobStarts < 3) && (HoldReasonCode == 34 || HoldReasonCode == 21) && (JobStatus == 5)' # periodic release for 3 times // Held reason is lack of memeory // JobStatus is Hold // https://research.cs.wisc.edu/htcondor/manual/v8.5/12_Appendix_A.html
}

# Group merges are independent processes writing independent outputs, which is
# the only concurrency shape ROOT's merger handles correctly. Parallelism comes
# from running many of these nodes, not from threads inside one, so one CPU is
# the right request.
partial_merge_job = {
    'universe': "vanilla",
    'executable': "merge_partial.sh",
    'JobBatchName': "batchname",
    '+singularityimage': f'"{SINGULARITY_IMAGE}"',
    'arguments': "$(Group)",
    'RequestCpus': 1,
    'RequestMemory': '4096 MB',
    'output': "merge_$(Group).out",
    'error': "merge_$(Group).err",
    'should_transfer_files': "YES",
    'when_to_transfer_output': "ON_EXIT_OR_EVICT",
    'periodic_release': '(NumJobStarts < 3) && (HoldReasonCode == 34 || HoldReasonCode == 21) && (JobStatus == 5)'
}

final_job = {
    'universe': "vanilla",
    'executable': "final.sh",
    'JobBatchName': "Summarize",
    'getenv': "True",
    'RequestMemory': '1024 MB',
    'output': "final.out",
    'error': "final.err",
    'should_transfer_files': "YES",
    'when_to_transfer_output': "ON_EXIT_OR_EVICT"
}
