clc;
clear;
%import danych z pliku
file_data = importdata('temp1.txt');
temp = file_data;
y0 = 25;
start_step = 80;
step_mult = 500;
max = 5000;
kp = 55;
ki = 0.08;
zad = 50;
temp = temp-y0;
file_data = importdata('sterowanie1.txt');
set_val = file_data;

%estymacja transmitancji obiektu
in_data = iddata(temp(start_step:end), set_val(start_step:end), 0.1);
my_tf = tfest(in_data, 3, 0, 'Ts', 0.1);


%wykres sygnalu z enkodera z usunieciem poczatkowych wartosci
t1 = 0:size(temp(start_step:end), 1)-1;
t1 = t1*0.1;
plot(t1, temp(start_step:end)+y0)
hold on

%wykres odpowiedzi skokowej transmitancji obiektu
[y, t] = step(my_tf);
y = y*step_mult+y0;
plot(t, y, 'r')