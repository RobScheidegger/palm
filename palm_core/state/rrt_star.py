def rrt_star(state, goal):
    """RRT* algorithm for finding a path from state to goal.
    Args:
        state (State): The starting state.
        goal (State): The goal state.
    Returns:
        path (list): A list of states from state to goal.
    """

    v = x_init
    E = []
    for i in range(n):
        x_rand = sample()
        x_near = nearest(x_rand, v)
        x_new = steer(x_near, x_rand)
        if collision_free(x_near, x_new):
            v_new = v + [x_new]
            E_new = E + [(x_near, x_new)]
            for x_near in v:
                if collision_free(x_near, x_new) and cost(x_near, x_new) < cost(x_near, x_nearest):
                    x_near_new = nearest(x_near, v_new)
                    E_new = E_new - [(x_near, x_near_new)] + [(x_near, x_new), (x_new, x_near_new)]
            v = v_new
            E = E_new
    return E