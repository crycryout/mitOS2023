### this is the answer to risc-v assembly in the trap lab
q1 : register a2 holds 13 when calling to printf
q2 : I didn't see the call to g or f in main, so maybe they are ommitted by
the compiler and replaced with a value.
q3 : we can find it in jalr 1554(ra), so it's the value of ra plus 1554
aupic is : ra = pc, so ra is 30 and the result would be 1584
q4 : I didn't run it , but I think it will show 57616 as a hex and print 
0x00646C72 as some ASCII , the details depends on the endianess of the architecture.
the question say that risc-v is little endian , so it will go like 72 6c 64 00
if it turns into big endian, then we need to reverse the hex's order,but the decimal would still remain the same.
q5 : well, I think in most of mordern compilers, this will result a warning or even an error.
if it does compiles, then the value of y depends on the number next to x in memory which is not initialized,
I think it is an undefined behavior. 
