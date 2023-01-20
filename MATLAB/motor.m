%import danych z pliku
file_data = importdata('SWV_Data_Trace.txt');
temp = file_data;
temp = temp-25;
file_data = importdata('SWV_Data_Trace1.txt');
set_val = file_data;

%estymacja transmitancji obiektu
in_data = iddata(temp(7:end), set_val(7:end), 0.1);
my_tf = tfest(in_data, 1, 0, 'Ts', 0.1);

%wykres sygnalu z enkodera z usunieciem poczatkowych wartosci
t1 = 0:size(temp(7:end), 1)-1;
t1 = t1*0.1;
plot(t1, temp(7:end))
hold on

%wykres odpowiedzi skokowej transmitancji obiektu
[y, t] = step(my_tf);
y = y*300;
plot(t, y, 'r')