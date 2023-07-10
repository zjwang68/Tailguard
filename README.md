# Tailguard
This page includes the source codes for ICDCS 2023 Tailguard paper

Mastree_cdf, Shore_cdf and Xapian_cdf store cdfs of the three qorkloads

RUT_Mastree, RUT_Shore, RUT_Xapian store unloaded query tail latencies with query fanout from 1 t0 1000 in the three workloads

Parameter set in case of impact of query fanout:  TNP=100; CLS=3; FAN=100; ATET=1.0; ARATE=x*111/3/0.176(Masstree), x*111/3/0.3413(Shore), x*111/3/0.9248 (Xapian), here x is the system load, e.g., x=0.6 if the load is 60%; PCT[CLS]={100./111., 110./111, 1.}

Parameter set in case of impact of service class:  TNP=100; CLS=2; FAN=100; ATET=1.0; ARATE=x/0.176(Masstree), x/0.3413(Shore), x/0.9248 (Xapian), x is the load; PCT[CLS]={0.5,1.0}
