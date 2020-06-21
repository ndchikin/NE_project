#!/usr/bin/env python

import os
import sys
from copy import deepcopy
from pysat.pb import PBEnc
from pysat.formula import CNF
from pysat.solvers import Solver


def toVar(player, a, b, n):
    return int(player * (n + 1) * n / 2 + a * (2 * n - a - 1) / 2 + b)


def toLit(player, a, b, n):
    if a < b:
        return toVar(player, a, b, n)
    else:
        return -toVar(player, b, a, n)


def atmost_for_each(g_size, dif, k, var = 0):
    ans = []
    if var == 0:
        var = int((dif * (g_size + 1) * g_size) / 2)
    for i in range(dif):
        lits = []
        for j in range(1, g_size + 1):
            lits.append(toLit(i, j, 0, g_size))
        ans += PBEnc.atmost(lits, bound = k, top_id = var).clauses
        for clause in ans:
            for lit in clause:
                var = max(abs(lit), var)
    return ans


def atmost_for_all(g_size, dif, k, var = 0):
    lits = []
    if var == 0:
        var = int((dif * (g_size + 1) * g_size) / 2)
    for i in range(dif):
        for j in range(1, g_size + 1):
            lits.append(toLit(i, j, 0, g_size))
    return PBEnc.atmost(lits, bound = k, top_id = var).clauses


def toOp(Lit, n, dif):
    if abs(Lit) > (dif * n * (n + 1)) / 2:
        return
    ans = [0] * 3
    aLit = abs(Lit) - 1
    num = int(((n + 1) * n) / 2)
    ans[0] = int(aLit / num)
    ab = aLit % num
    s = 0
    for i in range(n, 0, -1):
        s += i
        if ab < s:
            ans[1] = n - i
            ans[2] = n + ab - s + 1
            break
    if Lit < 0:
        ans[1], ans[2] = ans[2], ans[1]
    return ans


def toOut(num):
    return 'c' if num == 0 else 'a' + str(num)


def read_graph():
    graph = []
    try:
        size = int(sys.stdin.readline())
    except:
        sys.exit(0)
    for i in range(size):
        m = map(int, sys.stdin.readline().strip().split())
        num = next(m)
        graph.append(list(m))
    return graph


def write_graph(graph):
    size = len(graph)
    print(size, flush=True)
    for line in graph:
        print(len(line), ' '.join(map(str, line)), flush=True)


def read_vector():
    size = int(sys.stdin.readline())
    return list(map(int, sys.stdin.readline().strip().split()))


def write_vector(v):
    print(len(v), ' '.join(map(str, v)), flush=True)


def read_cnf():
    cnf = []
    size = int(sys.stdin.readline())
    for i in range(size):
        cnf.append(list(map(int, sys.stdin.readline().strip().split())))
    return cnf


def fing_min_for_all(cnf, g_size, dif, upper=-1):
    mid = 0
    lower = -1
    if upper == -1:
        upper = g_size * dif
    while lower + 1 != upper:
        mid = int((lower + upper) / 2)
        cur_cnf = cnf + atmost_for_all(g_size, dif, mid)
        solver = Solver('m22', cur_cnf)
        if solver.solve():
            model = deepcopy(solver.get_model())
            upper = mid
        else:
            lower = mid
        solver.delete()
    return upper, model
    

def fing_min_for_each(cnf, g_size, dif, upper=-1):
    mid = 0
    lower = -1
    if upper == -1:
        upper = g_size
    while lower + 1 != upper:
        mid = int((lower + upper) / 2)
        cur_cnf = cnf + atmost_for_each(g_size, dif, mid)
        solver = Solver('m22', cur_cnf)
        if solver.solve():
            model = deepcopy(solver.get_model())
            upper = mid
        else:
            lower = mid
        solver.delete()
    return upper, model


def make_orders(n, dif, model):
    
    ops = list(filter(lambda x: x, map(lambda x: toOp(x, n, dif), model)))
    count = [[0 for i in range(n + 1)] for j in range(dif)] 
    orders = [[0 for i in range(n + 1)] for j in range(dif)]

    for op in ops:
        count[op[0]][op[2]] += 1;

    for order, key in zip(orders, count):
        for outcome, pos in enumerate(key):
            order[pos] = outcome
    
    return orders


count = 0    
min_for_all = 1e10
min_for_each = 1e10

while True:
    
    graph = read_graph()
    players = read_vector()
    beg = int(sys.stdin.readline().strip())
    cnf = read_cnf()

    n = len(graph)
    dif = len(set(players))
    
    solver = Solver('m22', cnf)
    has_sol = solver.solve()
    solver.delete()

    if has_sol:

        new_min = False
       
        num, min_for_all_model = fing_min_for_all(cnf, n, dif)
        
        if num < min_for_all:
            new_min = True
            min_for_all = num
            
        num, min_for_each_model = fing_min_for_each(cnf, n, dif)
        
        if num < min_for_each:
            new_min = True
            min_for_each = num
            
        if new_min:
            write_graph(graph)
            print(flush=True)
            write_vector(players)
            print(flush=True)
            print(beg, flush=True)
            print(flush=True)
            print('min_for_all = %i:' % min_for_all, flush=True)
            print(flush=True)
            for order in make_orders(n, dif, min_for_all_model):
                print(' < '.join(map(toOut, order)), flush=True)
            print(flush=True)
            print('min_for_each = %i:' % min_for_each, flush=True)
            print(flush=True)
            for order in make_orders(n, dif, min_for_each_model):
                print(' < '.join(map(toOut, order)), flush=True)
            print(flush=True)
    
    count += 1
    
    if count % 100 == 0:
        print(count, end='\r', file=sys.stderr, flush=True)
