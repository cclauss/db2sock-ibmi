# command:     python run.py
import glob
import os
test_pgms = glob.glob("test*32")
test_pgms.sort()
for name in test_pgms:
  print("=============================")
  print(name)
  print("=============================")
  os.system("./" + name);

test_pgms = glob.glob("test*64")
test_pgms.sort()
for name in test_pgms:
  print("=============================")
  print(name)
  print("=============================")
  os.system("./" + name);

