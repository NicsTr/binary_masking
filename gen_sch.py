import string

def print_res(res):
    for i in res:
        print(''.join(i))

alph = string.digits + string.ascii_letters

d = int(input("d?\n"))

# Init S (the pairs of shares)
S = {}
for i in range(d + 1):
    for j in range(d + 1):
        if i == j:
            S[(i,j)] = "s{}{} ".format(alph[i],alph[i])
        else:
            S[(i,j)] = "s{0}{1} s{1}{0} ".format(alph[i],alph[j])

# Init R (the fresh random value)
R = []
for i in range(d**2):
    R.append("r{:02} ".format(i))

res = []
for i in range(d + 1):
    res.append([S.pop((i,i))])

j = 1
R2 = set()
while S:
    for i in range(d + 1):
        if j % 2:
            res[i] += R[((j - 1)//2)*(d+1) + i]
            R2.add(R[((j - 1)//2)*(d+1) + i])
        else:
            res[i] += R[((j - 2)//2)*(d+1) + (i + 1)%(d+1)]
            R2.remove(R[((j - 2)//2)*(d+1) + (i + 1)%(d+1)])
        
        if S:
            index = (i, (i+j)%(d+1))
            res[i] += S.pop(index)
            S.pop((index[1], index[0]))
        else:
            break
    j += 1

remain = len(R2)
for i in range(remain):
    res[i] += R[((j - 1)//2)*(d+1) + (i+1)%remain]
    


print_res(res)
