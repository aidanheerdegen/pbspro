/*
 * Copyright (C) 1994-2019 Altair Engineering, Inc.
 * For more information, contact Altair at www.altair.com.
 *
 * This file is part of the PBS Professional ("PBS Pro") software.
 *
 * Open Source License Information:
 *
 * PBS Pro is free software. You can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PBS Pro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial License Information:
 *
 * For a copy of the commercial license terms and conditions,
 * go to: (http://www.pbspro.com/UserArea/agreement.html)
 * or contact the Altair Legal Department.
 *
 * Altair’s dual-license business model allows companies, individuals, and
 * organizations to create proprietary derivative works of PBS Pro and
 * distribute them - whether embedded or bundled with other software -
 * under a commercial license agreement.
 *
 * Use of Altair’s trademarks, including but not limited to "PBS™",
 * "PBS Professional®", and "PBS Pro™" and Altair’s logos is subject to Altair's
 * trademark licensing policies.
 *
 */
/**
 * @file	get_ttr.c
 * @brief
 *      Locate an attribute (attrl) by name (and resource).
 */

#include <pbs_config.h>   /* the master config generated by configure */
#include "cmds.h"
#include "pbs_ifl.h"


/**
 * @brief
 *      Locate an attribute (attrl) by name (and resource).
 *
 * @param[in] pattrl    - Attribute list.
 * @param[in] name      - name to find in attribute list.
 * @param[in] resc      - resource to find in attribute list.
 *
 * @return	pointer to string
 * @retval      value of the located name and resource from attribute list,
 * @retval 	othewise NULL.
 */

char *
get_attr(struct attrl *pattrl, char *name, char *resc)
{
	while (pattrl) {
		if (strcmp(name, pattrl->name) == 0) {
			if (resc) {
				if (strcmp(resc, pattrl->resource) == 0) {
					return (pattrl->value);
				}
			} else {
				return (pattrl->value);
			}
		}
		pattrl = pattrl->next;
	}
	return NULL;
}

/*
 * @brief
 *	check_max_job_sequence_id - connect to the server and retrieve the
 *	                            max_job_sequence_id attribute value
 *
 *	@param[in]cmd_name - PBS command name
 *
 *	@retval  1	success
 *	@retval -1	error
 *
 */
int check_max_job_sequence_id(char *cmd_name)
{

	struct batch_status *server_attrs;
	int connect;
	char server_out[MAXSERVERNAME];
	server_out[0] = '\0';

	connect = cnt2server(server_out);
	if (connect <= 0) {
			fprintf(stderr, "%s: cannot connect to server (errno=%d)\n", cmd_name,
					pbs_errno);
			return -1;
	}
	server_attrs = pbs_statserver(connect, NULL, NULL);
	if (server_attrs == NULL) {
		if (pbs_errno) {
			char *errmsg;
			errmsg = pbs_geterrmsg(connect);
			if (errmsg != NULL)
				fprintf(stderr, "%s: %s\n", cmd_name, errmsg);
			else
				fprintf(stderr, "%s: Error (%d) getting status of server ", cmd_name, pbs_errno);
		}
		return -1;
	} else {
		struct attrl *attr;
		char * value;
		attr = server_attrs->attribs;
		value = get_attr(attr, ATTR_max_job_sequence_id, NULL);
		if (value == NULL) {
			pbs_statfree(server_attrs);
			pbs_disconnect(connect);
			/* if server is not configured for max_job_sequence_id
			* or attribute is unset */
			return 0;
		} else {
			/* if value is set */
			long long seq_id = 0;
			seq_id = strtoul(value, NULL, 10);
			if (seq_id > PBS_DFLT_MAX_JOB_SEQUENCE_ID) {
				pbs_statfree(server_attrs);
				pbs_disconnect(connect);
				return 1;
			}
			return 0;
		}
	}
}
