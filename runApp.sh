if gcc $(pkg-config --cflags gtk4) -o application application.c $(pkg-config --libs gtk4) 
then
# gcc $(pkg-config --cflags gtk4) -S -o app.s application.c 
./application
fi