/*
 * Copyright (c) 2005 Topspin Communications.  All rights reserved.
 * Copyright (c) 2005 Mellanox Technologies Ltd.  All rights reserved.
 * Copyright (c) 2005 Hewlett Packard, Inc (Grant Grundler)
 * Copyright (c) 2009 HNR Consulting.  All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * $Id$
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if !defined(__FreeBSD__)
#include <malloc.h>
#endif

#include "get_clock.h"
#include "perftest_parameters.h"
#include "perftest_resources.h"
#include "multicast_resources.h"
#include "perftest_communication.h"

/******************************************************************************
 *
 ******************************************************************************/
static int set_mcast_group(struct pingpong_context *ctx,
		struct perftest_parameters *user_param,
		struct mcast_parameters *mcg_params)
{
	struct ibv_port_attr port_attr;

	if (ibv_query_gid(ctx->context,user_param->ib_port,user_param->gid_index,&mcg_params->port_gid)) {
		return FAILURE;
	}

	if (ibv_query_pkey(ctx->context,user_param->ib_port,DEF_PKEY_IDX,&mcg_params->pkey)) {
		return FAILURE;
	}

	if (ibv_query_port(ctx->context,user_param->ib_port,&port_attr)) {
		return FAILURE;
	}
	mcg_params->sm_lid  = port_attr.sm_lid;
	mcg_params->sm_sl   = port_attr.sm_sl;
	mcg_params->ib_port = user_param->ib_port;
	mcg_params->ib_ctx  = ctx->context;
	mcg_params->ib_devname = user_param->ib_devname;

	if (!strcmp(link_layer_str(user_param->link_type),"IB")) {
		/* Request for Mcast group create registery in SM. */
		if (join_multicast_group(SUBN_ADM_METHOD_SET,mcg_params)) {
			fprintf(stderr,"Couldn't Register the Mcast group on the SM\n");
			return FAILURE;
		}
	}
	return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
static int send_set_up_connection(struct pingpong_context *ctx,
		struct perftest_parameters *user_param,
		struct pingpong_dest *my_dest,
		struct mcast_parameters *mcg_params,
		struct perftest_comm *comm)
{

	if (set_up_connection(ctx,user_param,my_dest)) {
		fprintf(stderr," Unable to set up my IB connection parameters\n");
		return FAILURE;
	}

	if (user_param->use_mcg) {
		int i;
		mcg_params->user_mgid = user_param->user_mgid;

		set_multicast_gid(mcg_params,ctx->qp[0]->qp_num,(int)user_param->machine);
		if (set_mcast_group(ctx,user_param,mcg_params)) {
			return FAILURE;
		}

		for (i=0; i < user_param->num_of_qps; i++) {
			if (ibv_attach_mcast(ctx->qp[i],&mcg_params->mgid,mcg_params->mlid)) {
				fprintf(stderr, "Couldn't attach QP to MultiCast group\n");
				return FAILURE;
			}
		}

		mcg_params->mcast_state |= MCAST_IS_ATTACHED;
		my_dest->gid = mcg_params->mgid;
		my_dest->lid = mcg_params->mlid;
		my_dest->qpn = QPNUM_MCAST;
	}
	return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
static int send_destroy_ctx(struct pingpong_context *ctx,
		struct perftest_parameters *user_param,
		struct mcast_parameters *mcg_params)
{
	if (user_param->use_mcg) {
		int i;
		for (i=0; i < user_param->num_of_qps; i++) {
				if (ibv_detach_mcast(ctx->qp[i],&mcg_params->base_mgid,mcg_params->base_mlid)) {
					fprintf(stderr, "Couldn't dettach QP to MultiCast group\n");
					return FAILURE;
				}
		}

		if (!strcmp(link_layer_str(user_param->link_type),"IB")) {
			if (join_multicast_group(SUBN_ADM_METHOD_DELETE,mcg_params)) {
				fprintf(stderr,"Couldn't Unregister the Mcast group on the SM\n");
				return FAILURE;
			}

			memcpy(mcg_params->mgid.raw,mcg_params->base_mgid.raw,16);

			if (join_multicast_group(SUBN_ADM_METHOD_DELETE,mcg_params)) {
				fprintf(stderr,"Couldn't Unregister the Mcast group on the SM\n");
				return FAILURE;
			}
		}
	}
	return destroy_ctx(ctx,user_param);
}

/******************************************************************************
 *
 ******************************************************************************/
int main(int argc, char *argv[])
{
	int                        i = 0, rc, error = 1;
	int                        size_max_pow = 24;
	int			   ret_val;
	struct report_options      report;
	struct pingpong_context    ctx;
	struct pingpong_dest	   *my_dest  = NULL;
	struct pingpong_dest	   *rem_dest = NULL;
	struct mcast_parameters	   mcg_params;
	struct ibv_device          *ib_dev = NULL;
	struct perftest_parameters user_param;
	struct perftest_comm	   user_comm;
	int rdma_cm_flow_destroyed = 0;

	/* init default values to user's parameters */
	memset(&ctx,		0, sizeof(struct pingpong_context));
	memset(&user_param, 0, sizeof(struct perftest_parameters));
	memset(&user_comm , 0, sizeof(struct perftest_comm));
	memset(&mcg_params, 0, sizeof(struct mcast_parameters));

	user_param.verb    = SEND;
	user_param.tst     = LAT;
	strncpy(user_param.version, VERSION, sizeof(user_param.version));
	user_param.r_flag  = &report;

	/* Configure the parameters values according to user arguments or defalut values. */
	ret_val = parser(&user_param,argv,argc);
	if (ret_val) {
		if (ret_val != VERSION_EXIT && ret_val != HELP_EXIT)
			fprintf(stderr," Parser function exited with Error\n");
		goto return_error;
	}

	if(user_param.use_xrc || user_param.connection_type == DC) {
		user_param.num_of_qps *= 2;
	}

	/* Checking that the user did not run with RawEth. for this we have raw_etherent_bw test. */
	if (user_param.connection_type == RawEth) {
		fprintf(stderr," This test cannot run Raw Ethernet QPs (you have chosen RawEth as connection type\n");
		goto return_error;
	}

	/* Finding the IB device selected (or defalut if no selected). */
	ib_dev = ctx_find_dev(&user_param.ib_devname);
	if (!ib_dev) {
		fprintf(stderr," Unable to find the Infiniband/RoCE device\n");
		goto return_error;
	}

	/* Getting the relevant context from the device */
	ctx.context = ctx_open_device(ib_dev, &user_param);
	if (!ctx.context) {
		fprintf(stderr, " Couldn't get context for the device\n");
		goto free_devname;
	}

	/* Verify user parameters that require the device context,
	 * the function will print the relevent error info. */
	if (verify_params_with_device_context(ctx.context, &user_param)) {
		goto free_devname;
	}

	/* See if link type is valid and supported. */
	if (check_link(ctx.context,&user_param)) {
		fprintf(stderr, " Couldn't get context for the device\n");
		goto free_devname;
	}

	/* copy the relevant user parameters to the comm struct + creating rdma_cm resources. */
	if (create_comm_struct(&user_comm,&user_param)) {
		fprintf(stderr," Unable to create RDMA_CM resources\n");
		goto free_devname;
	}

	if (user_param.output == FULL_VERBOSITY && user_param.machine == SERVER) {
		printf("\n************************************\n");
		printf("* Waiting for client to connect... *\n");
		printf("************************************\n");
	}

	/* Initialize the connection and print the local data. */
	if (establish_connection(&user_comm)) {
		fprintf(stderr," Unable to init the socket connection\n");
		dealloc_comm_struct(&user_comm,&user_param);
		goto free_devname;
	}

	exchange_versions(&user_comm, &user_param);
	check_version_compatibility(&user_param);
	check_sys_data(&user_comm, &user_param);

	/* See if MTU is valid and supported. */
	if (check_mtu(ctx.context,&user_param, &user_comm)) {
		fprintf(stderr, " Couldn't get context for the device\n");
		dealloc_comm_struct(&user_comm,&user_param);
		return FAILURE;
	}

	MAIN_ALLOC(my_dest , struct pingpong_dest , user_param.num_of_qps , free_rdma_params);
	memset(my_dest, 0, sizeof(struct pingpong_dest)*user_param.num_of_qps);
	MAIN_ALLOC(rem_dest , struct pingpong_dest , user_param.num_of_qps , free_my_dest);
	memset(rem_dest, 0, sizeof(struct pingpong_dest)*user_param.num_of_qps);

	/* Allocating arrays needed for the test. */
	if(alloc_ctx(&ctx,&user_param)){
		fprintf(stderr, "Couldn't allocate context\n");
		goto free_mem;
	}

	/* Create RDMA CM resources and connect through CM. */
	if (user_param.work_rdma_cm == ON) {
		rc = create_rdma_cm_connection(&ctx, &user_param, &user_comm,
			my_dest, rem_dest);
		if (rc) {
			fprintf(stderr,
				"Failed to create RDMA CM connection with resources.\n");
			dealloc_ctx(&ctx, &user_param);
			goto free_mem;
		}
	} else {
		/* create all the basic IB resources (data buffer, PD, MR, CQ and events channel) */
		if (ctx_init(&ctx, &user_param)) {
			fprintf(stderr, " Couldn't create IB resources\n");
			dealloc_ctx(&ctx, &user_param);
			goto free_mem;
		}
	}

	/* Set up the Connection. */
	if (send_set_up_connection(&ctx,&user_param,my_dest,&mcg_params,&user_comm)) {
		fprintf(stderr," Unable to set up socket connection\n");
		goto destroy_ctx;
	}

	/* Print basic test information. */
	ctx_print_test_info(&user_param);
	check_bf_support(&ctx);

	for (i=0; i < user_param.num_of_qps; i++) {

		/* shaking hands and gather the other side info. */
		if (ctx_hand_shake(&user_comm,&my_dest[i],&rem_dest[i])) {
			fprintf(stderr,"Failed to exchange data between server and clients\n");
			goto destroy_ctx;
		}
	}

	if (user_param.work_rdma_cm == OFF) {
		if (ctx_check_gid_compatibility(&my_dest[0], &rem_dest[0])) {
			fprintf(stderr,"\n Found Incompatibility issue with GID types.\n");
			fprintf(stderr," Please Try to use a different IP version.\n\n");
			goto destroy_ctx;
		}
	}

	if (user_param.use_mcg) {
		memcpy(mcg_params.base_mgid.raw,mcg_params.mgid.raw,16);
		memcpy(mcg_params.mgid.raw,rem_dest[0].gid.raw,16);
		mcg_params.base_mlid = mcg_params.mlid;
		mcg_params.is_2nd_mgid_used = ON;
		if (!strcmp(link_layer_str(user_param.link_type),"IB")) {
			/* Request for Mcast group create registery in SM. */
			if (join_multicast_group(SUBN_ADM_METHOD_SET,&mcg_params)) {
				fprintf(stderr," Failed to Join Mcast request\n");
				goto destroy_ctx;
			}
		}

		/*
		 * The next stall in code (50 ms sleep) is a work around for fixing the
		 * the bug this test had in Multicast for the past 1 year.
		 * It appears, that when a switch involved, it takes ~ 10 ms for the join
		 * request to propogate on the IB fabric, thus we need to wait for it.
		 * what happened before this fix was  reaching the post_send
		 * code segment in about 350 ns from here, and the switch(es) dropped
		 * the packet because join request wasn't finished.
		 */
		usleep(50000);
	}

	if (user_param.work_rdma_cm == OFF) {

		/* Prepare IB resources for rtr/rts. */
		if (ctx_connect(&ctx,rem_dest,&user_param,my_dest)) {
			fprintf(stderr," Unable to Connect the HCA's through the link\n");
			goto destroy_ctx;
		}
	}

	/* shaking hands and gather the other side info. */
	if (ctx_hand_shake(&user_comm,&my_dest[0],&rem_dest[0])) {
		fprintf(stderr,"Failed to exchange data between server and clients\n");
		goto destroy_ctx;
	}

	if (user_param.connection_type == DC)
	{
		/* Set up connection one more time to send qpn properly for DC */
		if (set_up_connection(&ctx, &user_param, my_dest))
		{
			fprintf(stderr, " Unable to set up socket connection\n");
			goto destroy_ctx;
		}
	}

	/* Print this machine QP information */
	for (i=0; i < user_param.num_of_qps; i++)
		ctx_print_pingpong_data(&my_dest[i],&user_comm);

	user_comm.rdma_params->side = REMOTE;

	for (i=0; i < user_param.num_of_qps; i++) {
		if (ctx_hand_shake(&user_comm,&my_dest[i],&rem_dest[i])) {
			fprintf(stderr," Failed to exchange data between server and clients\n");
			goto destroy_ctx;
		}

		ctx_print_pingpong_data(&rem_dest[i],&user_comm);
	}

	if (user_param.use_event) {

		if (ibv_req_notify_cq(ctx.send_cq, 0)) {
			fprintf(stderr, "Couldn't request RCQ notification\n");
			goto destroy_ctx;
		}

		if (ibv_req_notify_cq(ctx.recv_cq, 0)) {
			fprintf(stderr, "Couldn't request RCQ notification\n");
			goto destroy_ctx;
		}
	}
	if (user_param.output == FULL_VERBOSITY) {
		printf(RESULT_LINE);
		printf("%s",(user_param.test_type == ITERATIONS) ? RESULT_FMT_LAT : RESULT_FMT_LAT_DUR);
		printf((user_param.cpu_util_data.enable ? RESULT_EXT_CPU_UTIL : RESULT_EXT));
	}

	ctx_set_send_wqes(&ctx,&user_param,rem_dest);

	if (user_param.test_method == RUN_ALL) {
		if (user_param.connection_type == UD)
			size_max_pow = (int)MSG_SZ_2_EXP(MTU_SIZE(user_param.curr_mtu)) + 1;
		else if (user_param.connection_type == SRD)
			size_max_pow = (int)MSG_SZ_2_EXP(user_param.size) + 1;

		for (i = 1; i < size_max_pow ; ++i) {

			user_param.size = (uint64_t)1 << i;

			/* Post receive recv_wqes fo current message size */
			if (ctx_set_recv_wqes(&ctx,&user_param)) {
				fprintf(stderr," Failed to post receive recv_wqes\n");
				goto free_mem;
			}

			/* Sync between the client and server so the client won't send packets
			 * Before the server has posted his receive wqes (in UC/UD it will result in a deadlock).
			 */

			if (ctx_hand_shake(&user_comm,&my_dest[0],&rem_dest[0])) {
				fprintf(stderr,"Failed to exchange data between server and clients\n");
				goto free_mem;
			}

			if(run_iter_lat_send(&ctx, &user_param)){
				error = 17;
				goto free_mem;
			}

			user_param.test_type == ITERATIONS ? print_report_lat(&user_param) : print_report_lat_duration(&user_param);
		}

	} else {

		/* Post recevie recv_wqes fo current message size */
		if (ctx_set_recv_wqes(&ctx,&user_param)) {
			fprintf(stderr," Failed to post receive recv_wqes\n");
			goto free_mem;
		}

		/* Sync between the client and server so the client won't send packets
		 * Before the server has posted his receive wqes (in UC/UD it will result in a deadlock).
		 */

		if (ctx_hand_shake(&user_comm,my_dest,rem_dest)) {
			fprintf(stderr,"Failed to exchange data between server and clients\n");
			goto free_mem;
		}

		if(run_iter_lat_send(&ctx, &user_param)){
			error = 17;
			goto free_mem;
		}

		user_param.test_type == ITERATIONS ? print_report_lat(&user_param) : print_report_lat_duration(&user_param);
	}

	if (user_param.output == FULL_VERBOSITY) {
		printf(RESULT_LINE);
	}

	if (ctx_close_connection(&user_comm,my_dest,rem_dest)) {
		fprintf(stderr,"Failed to close connection between server and client\n");
		fprintf(stderr," Trying to close this side resources\n");
	}

	if (user_param.work_rdma_cm == ON) {
		if (send_destroy_ctx(&ctx,&user_param,&mcg_params)) {
			fprintf(stderr, "Failed to destroy resources\n");
			goto destroy_cm_context;
		}
		user_comm.rdma_params->work_rdma_cm = OFF;
		free(rem_dest);
		free(my_dest);
		free(user_param.ib_devname);
		if(destroy_ctx(user_comm.rdma_ctx, user_comm.rdma_params)) {
			free(user_comm.rdma_params);
			free(user_comm.rdma_ctx);
			return FAILURE;
		}
		free(user_comm.rdma_params);
		free(user_comm.rdma_ctx);
		return SUCCESS;
	}

	free(rem_dest);
	free(my_dest);
	free(user_param.ib_devname);

	return send_destroy_ctx(&ctx,&user_param,&mcg_params);

destroy_ctx:
	if (send_destroy_ctx(&ctx,&user_param,&mcg_params))
		fprintf(stderr, "Failed to destroy resources\n");

destroy_cm_context:
	if (user_param.work_rdma_cm == ON) {
		rdma_cm_flow_destroyed = 1;
		user_comm.rdma_params->work_rdma_cm = OFF;
		destroy_ctx(user_comm.rdma_ctx,user_comm.rdma_params);
	}

free_mem:
	free(rem_dest);
free_my_dest:
	free(my_dest);
free_rdma_params:
	if (user_param.use_rdma_cm == ON && rdma_cm_flow_destroyed == 0)
		dealloc_comm_struct(&user_comm, &user_param);
	else {
		if(user_param.use_rdma_cm == ON)
			free(user_comm.rdma_ctx);
		free(user_comm.rdma_params);
	}
free_devname:
	free(user_param.ib_devname);
return_error:
	//coverity[leaked_storage]
	return error;
}
