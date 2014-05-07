'''
Created on 26 mars 2014

@author: d.amar
'''

if __name__ == '__main__':
    ass={}
    ass["test"]="ttt"
    ass["test2"]="sss"
    key = "test"
    if key in ass:
        print ass[key]
    else:
        print "not here"