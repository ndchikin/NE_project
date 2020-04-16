import os
import tqdm
from pysat.pb import PBEnc
from pysat.formula import CNF
from pysat.solvers import Solver

ans1 = [False] * 10
ans2 = [False] * 10

for i in range(3, 10):
    for j in range(1, i + 1):
        print(i, j)
        DATA_p = 'PPH/' + str(i) + '/' + str(j) + '/'
        DATA_f = os.listdir(DATA_p)
        cnf_t = CNF('PPH/' + str(i) + '/' + str(j) + '/ter.txt').clauses
        for f in tqdm.tqdm(DATA_f):
            if f != 'ter.txt':
                cnf = cnf_t + CNF(DATA_p + f).clauses
                solver = Solver('g4', cnf)
                if solver.solve():
#                     os.remove(DATA_p + f)
                    ans1[i] = True
                else:
                    ans2[i] = True
                solver.delete()
            else:
                print("TER")

print(ans1)
print(ans2)
