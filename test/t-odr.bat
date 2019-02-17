cl -EHsc -DMAIN=""             -Dnsel_P0323R=-2 -I../include -Foodr1.obj -c odr.cpp
cl -EHsc -DMAIN="int main(){}" -Dnsel_P0323R=-2 -I../include -Foodr2.obj -c odr.cpp
cl odr1.obj odr2.obj
