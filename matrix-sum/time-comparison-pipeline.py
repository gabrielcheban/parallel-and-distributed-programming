import sys
import os

linhas = sys.argv[1]
colunas = sys.argv[2]

os.system("python2 gera_matriz.py "+linhas+" "+colunas+" A.txt")
os.system("./seq A.txt A.txt "+linhas+" "+colunas)
os.system("mpirun -np 4 parallel A.txt A.txt "+linhas+" "+colunas)
