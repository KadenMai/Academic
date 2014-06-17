///////////////////////////////////////////////////////////
/// Student: Vu Pham
/// Project: Bucket Sort
///////////////////////////////////////////////////////////

#include "stdafx.h"
#include <iostream>
#include <omp.h>
#include <time.h>
#define n 10000000
#define bsize 1000000
#define m 100

int list[n];
int final[n];
int bucket[m][bsize];
int bcount[m];

int i, j, maxval, minval, range, bnum, ncount;
omp_lock_t slock, alock[m];
clock_t tstart, tsquential, tparallel, tdistribution;

using namespace std;

int lt(const void *p, const void *q);
int hf(int i);
void init();
void v0();
void v1();
void v2();
void v3();

int _tmain(int argc, _TCHAR* argv[])
{
	//////////////////////INIT DATA/////////////////////////////
	init();
	//////////////////////SEQUENTIAL VERSION////////////////////
	v0();
	//////////////////////PARALLEL VESION 1: SPINLOCK///////////
	v1();
	//////////////////////PARALLEL VESION 2: FINE-GRAIN/////////
	v2();
	//////////////////////PARALLEL VESION 3: LOCK-FREE//////////
	v3();
	////////////////////////////////////////////////////////////
	getchar();
	return 0;
}

/// Compare 2 integers
/// Return: 
///			+ p < q : -1
///			+ p = q : 0
///			+ p > q : 1
int lt(const void *p, const void *q) {
	return (*(int *)p - *(int *)q);
}

/// Determine destination bucket for list[i]
int hf(int i) {
	return (int)((float)m * ((float)(list[i] - minval) / range));
}

/// Initialize data 
void init() {
	cout << "============INITIALIZE DATA=======================" << endl;
	cout << "NUMBER OF PROCESSORS: " << omp_get_num_procs() << endl << endl;
	for (i = 0; i < n; i++) list[i] = rand();
	maxval = minval = list[0];
	for (i = 1; i < n; i++) {
		if (list[i] > maxval) maxval = list[i];
		if (list[i] < minval) minval = list[i];
	}
	range = maxval - minval + 1;
}

/// Version 0: Sequential
void v0() {
	// phase 1
	tstart = clock();
	ncount = 0;
	for (i = 0; i < n; i++) {
		bnum = hf(i);
		bucket[bnum][bcount[bnum]++] = list[i];
	}
	tdistribution = clock() - tstart;

	// phase 2
	for (i = 0; i < m; i++) {
		qsort(bucket[i], bcount[i], sizeof(int), lt);
		for (j = 0; j < bcount[i]; j++)
			final[ncount++] = bucket[i][j];
	}
	tsquential = clock() - tstart;

	cout << "============SEQUENTIAL VERSION====================" << endl;
	cout << "DISTRIBUTION PHASE TIME: " << tdistribution << endl;
	cout << "EXECUTION TIME: " << tsquential << endl << endl;
}

/// Version 1: Single spinlock
void v1() {
	// reset data
	for (i = 0; i < m; i++) bcount[i] = 0;
	for (i = 0; i < n; i++) final[i] = 0;

	// phase 1
	tstart = clock();
	omp_set_num_threads(omp_get_num_procs());
	omp_init_lock(&slock);
#pragma omp parallel for private(bnum)
	for (i = 0; i < n; i++) {
		bnum = hf(i);
		omp_set_lock(&slock);
		bucket[bnum][bcount[bnum]++] = list[i];
		omp_unset_lock(&slock);
	}
	tdistribution = clock() - tstart;

	// phase 2
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
	for (i = 0; i < m; i++)
		qsort(bucket[i], bcount[i], sizeof(int), lt);

	// phase 3
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for private(ncount, j)
	for (i = 0; i < m; i++) {
		ncount = 0;
		for (j = 0; j < i; j++)
			ncount += bcount[j];
		for (j = 0; j < bcount[i]; j++)
			final[ncount++] = bucket[i][j];
	}
	tparallel = clock() - tstart;
	cout << "============PARALLEL VERSION 1: SPINLOCK==========" << endl;
	cout << "DISTRIBUTION PHASE TIME: " << tdistribution << endl;
	cout << "EXECUTION TIME: " << tparallel << endl;
	cout << "SPEEDUP: " << (float)tsquential / tparallel << endl;
	cout << "PROCESSOR UTILIZATION: " <<
		(float)tsquential / tparallel / omp_get_num_procs() << endl << endl;

}

/// Version 2: Fine-grain locking (multiple lock)
void v2() {
	for (i = 0; i < m; i++) bcount[i] = 0;
	for (i = 0; i < n; i++) final[i] = 0;

	// phase 1
	tstart = clock();
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel
	for (i = 0; i < m; i++) omp_init_lock(&alock[i]);
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for private(bnum)
	for (i = 0; i < n; i++) {
		bnum = hf(i);
		omp_set_lock(&alock[bnum]);
		bucket[bnum][bcount[bnum]++] = list[i];
		omp_unset_lock(&alock[bnum]);
	}
	tdistribution = clock() - tstart;

	// phase 2
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
	for (i = 0; i < m; i++)
		qsort(bucket[i], bcount[i], sizeof(int), lt);

	// phase 3
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for private(ncount, j)
	for (i = 0; i < m; i++) {
		ncount = 0;
		for (j = 0; j < i; j++)
			ncount += bcount[j];
		for (j = 0; j < bcount[i]; j++)
			final[ncount++] = bucket[i][j];
	}
	tparallel = clock() - tstart;
	cout << "============PARALLEL VERSION 2: FINE-GRAIN========" << endl;
	cout << "DISTRIBUTION PHASE TIME: " << tdistribution << endl;
	cout << "EXECUTION TIME: " << tparallel << endl;
	cout << "SPEEDUP: " << (float)tsquential / tparallel << endl;
	cout << "PROCESSOR UTILIZATION: " <<
		(float)tsquential / tparallel / omp_get_num_procs() << endl << endl;
}
/// Version 3: Lock-free parallel implementation
void v3() {
	for (i = 0; i < m; i++) bcount[i] = 0;
	for (i = 0; i < n; i++) final[i] = 0;

	// phase 1
	tstart = clock();
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for private(j, bnum)
	for (i = 0; i < omp_get_num_procs(); i++) {
		int start = i * (m / omp_get_num_procs());
		int end = (i + 1) * (m / omp_get_num_procs()) - 1;
		for (j = 0; j < n; j++) {
			bnum = hf(j);
			if (bnum >= start && bnum <= end)
				bucket[bnum][bcount[bnum]++] = list[j];
		}
	}
	tdistribution = clock() - tstart;

	// phase 2
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for
	for (i = 0; i < m; i++)
		qsort(bucket[i], bcount[i], sizeof(int), lt);

	// phase 3
	omp_set_num_threads(omp_get_num_procs());
#pragma omp parallel for private(ncount, j)
	for (i = 0; i < m; i++) {
		ncount = 0;
		for (j = 0; j < i; j++)
			ncount += bcount[j];
		for (j = 0; j < bcount[i]; j++)
			final[ncount++] = bucket[i][j];
	}
	tparallel = clock() - tstart;
	cout << "============PARALLEL VERSION 3: LOCK-FREE=========" << endl;
	cout << "DISTRIBUTION PHASE TIME: " << tdistribution << endl;
	cout << "EXECUTION TIME: " << tparallel << endl;
	cout << "SPEEDUP: " << (float)tsquential / tparallel << endl;
	cout << "PROCESSOR UTILIZATION: " <<
		(float)tsquential / tparallel / omp_get_num_procs() << endl << endl;
}


