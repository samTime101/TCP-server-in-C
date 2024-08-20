"""
TODO:polish the UI and make it good
Author : samip Regmi
file:client.py
GUI for the client code implemented in C
"""

from tkinter import *
import socket
import threading

MAX  = 1024
# recv func connected  to our thread
def recv_func(client_socket, text_area):
    while True:
        try:
            rcv = client_socket.recv(MAX).decode('utf-8')
            if not rcv:
                break
            text_area.insert(END, rcv)
            text_area.yview(END)
        except:
            text_area.insert(END, "Error receiving message.\n")
            break
    client_socket.close()

#so we send our message to all the clients and that;s all
def send_func(client_socket, message):
    client_socket.sendall(message.encode('utf-8'))
    if message == "exit\n":
        client_socket.close()

# send what ever we wrote on form to send function
def on_send_click(client_socket,text_area,form):
    message = f"{form.get()}\n"
    send_func(client_socket, message)

def connect_client(port,address):
    log.destroy()
    """
    New main window
    """
    window = Tk()
    window.title("Chat Client")
    window.attributes("-zoomed",True)

    port_label_window = Label(window,text=port)
    port_label_window.pack()

    address_label_window = Label(window,text=address)
    address_label_window.pack()

    text_area = Text(window, height=30, width=100)
    text_area.pack(pady=10)
    text_area.config(state=NORMAL)

    label = Label(window,text = "cHAT ROOM \n Made by SamipRegmi\n*****\ncommands:\nhelp-info\nhelp-cc")
    label.place(x = 100,y=50)

    form = Entry(window, width=40)
    form.pack(pady=10)

    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((address, port))

    recv_thread = threading.Thread(target=recv_func, args=(client_socket, text_area))
    recv_thread.daemon = True
    recv_thread.start()

    connect = Button(window, text="Send", command=lambda: on_send_click(client_socket, text_area, form))
    connect.pack(pady=5)

    window.mainloop()

def connected():
    port_text = port.get()
    address_text  = address.get()
    port_text = int(port_text)
    connect_client(port_text,address_text)

# main info window
if __name__ == "__main__":
	log = Tk()
	log.title("Chat Client")
	log.attributes("-zoomed",True)

	info_label = Label(log,text = "GUI CLIENT\nMade by Samip Regmi\nSamTime101")
	info_label.place(x=600,y=50)

	port_label = Label(log,text="PORT")
	port_label.place(x=400,y=300)

	port = Entry(log, width=10)
	port.place(x=450,y=300)

	address_label = Label(log,text="ADDRESS")
	address_label.place(x=550,y=300)


	address = Entry(log , width=40)
	address.place(x=650,y=300)

	port_text = port.get()
	address_text  = address.get()

	send_button = Button(log, text="CONNECT",command=connected)
	send_button.place(x=580,y=350)

	log.mainloop()



