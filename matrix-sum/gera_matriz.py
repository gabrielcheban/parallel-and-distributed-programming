import numpy as np
import sys

# recebe como argumentos a quantidade de linhas e colunas
n_linhas = int(sys.argv[1])
n_colunas = int(sys.argv[2])

# recebe como argumento o nome do arquivo
arquivo = sys.argv[3]

# gera matriz
matriz = np.random.rand(n_linhas, n_colunas) * 753

#salva no arquivo texto
np.savetxt(arquivo, matriz, fmt='%.0f')
