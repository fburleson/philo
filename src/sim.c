/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sim.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: joel <joel@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/28 14:49:47 by joel              #+#    #+#             */
/*   Updated: 2023/05/02 22:21:13 by joel             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>

#include <stdlib.h>
#include "philo.h"

void	start_sim(t_sim *sim)
{
	pthread_mutex_lock(sim->sync_mutex);
	sim->is_running = TRUE;
	sim->time_start = get_time();
	pthread_mutex_unlock(sim->sync_mutex);
}

void	clean_sim(t_sim *sim)
{
	unsigned int	cidx;

	cidx = 0;
	while (cidx < sim->n_philo)
	{
		pthread_mutex_destroy(*(sim->forks + cidx));
		free(*(sim->philosophers + cidx));
		free(*(sim->forks + cidx));
		cidx++;
	}
	pthread_mutex_destroy(sim->sync_mutex);
	pthread_mutex_destroy(sim->log_mutex);
	free(sim->philosophers);
	free(sim->forks);
}

void	wait_for_threads(t_sim *sim)
{
	unsigned int	cidx;
	t_philosopher	*philosopher;

	cidx = 0;
	while (cidx < sim->n_philo)
	{
		philosopher = *(sim->philosophers + cidx);
		pthread_join(philosopher->thread_id, NULL);
		cidx++;
	}
}

void	check_meal_quota_met(t_sim *sim)
{
	t_philosopher	*philosopher;
	unsigned int	cidx;

	cidx = 0;
	while (cidx < sim->n_philo)
	{
		philosopher = *(sim->philosophers + cidx);
		if (!philosopher->is_finished)
			return ;
		cidx++;
	}
	pthread_mutex_lock(sim->sync_mutex);
	sim->should_stop = TRUE;
	pthread_mutex_unlock(sim->sync_mutex);
	wait_for_threads(sim);
	sim->is_running = FALSE;
	return ;
}

void	check_starvation(t_sim *sim)
{
	unsigned int	cidx;
	t_philosopher	*philosopher;

	cidx = 0;
	while (cidx < sim->n_philo)
	{
		philosopher = *(sim->philosophers + cidx);
		pthread_mutex_lock(philosopher->philo_mutex);
		if (get_timestamp(sim->time_start) - philosopher->last_meal
			> sim->time_die)
		{
			pthread_mutex_unlock(philosopher->philo_mutex);
			log_state(philosopher->id, DIE, sim);
			pthread_mutex_lock(sim->sync_mutex);
			sim->should_stop = TRUE;
			pthread_mutex_unlock(sim->sync_mutex);
			wait_for_threads(sim);
			sim->is_running = FALSE;
			return ;
		}
		else
			pthread_mutex_unlock(philosopher->philo_mutex);
		cidx++;
	}
}

unsigned int	should_sim_stop(t_sim *sim)
{
	unsigned int	should_sim_stop;

	pthread_mutex_lock(sim->sync_mutex);
	should_sim_stop = sim->should_stop;
	pthread_mutex_unlock(sim->sync_mutex);
	return (should_sim_stop);
}
