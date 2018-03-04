import glob
import numpy as np
def analysis_logs(log_path):
    lognames = glob.glob(log_path+'*')
    print(log_path,lognames)
    results = [np.loadtxt(logname,delimiter=',')[:,1] for logname in lognames]
    return np.array(results)

if __name__ == '__main__':
    import os
    print(os.getcwd())
    import sys
    logname = sys.argv[1]
    dataname = sys.argv[2]

    np.savetxt(dataname, analysis_logs(logname).T, delimiter=',')



