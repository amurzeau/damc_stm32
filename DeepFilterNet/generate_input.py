import numpy as np

emb = np.random.uniform(size=(1, 1, 512))
c0 = np.random.uniform(size=(1, 64, 1, 96))

print("model_data_type_t emb[1][1][512] = {{")
for i in range(512):
    value = emb[0][0][i].astype(np.float16)
    print(f"{value:e}", end="")
    if (i + 1) % 6 == 0:
        print(",\n", end="")
    else:
        print(",  ", end="")
print("}};")

print("model_data_type_t c0[1][64][1][96] = {{")
for i in range(64):
    print("{{")
    for j in range(96):
        value = c0[0][i][0][j].astype(np.float16)
        print(f"{value:e}", end="")
        if (j + 1) % 6 == 0:
            print(",\n", end="")
        else:
            print(",  ", end="")
    print("}},")
print("}};")
