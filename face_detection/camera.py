import socket
import cv2

# socket settings
HOST = '192.168.104.5'
PORT = 3000

# opencv settings
xml = 'haarcascade_frontalface_default.xml'
face_cascade = cv2.CascadeClassifier(xml)
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)

def camera_module():
    while(True):
        ret, frame = cap.read()
        frame = cv2.flip(frame, 1)
        frame = cv2.rotate(frame, cv2.ROTATE_180)
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

        faces = face_cascade.detectMultiScale(gray, 1.05, 5)
        print("faces detection: YES" if len(faces) >= 1 else "faces detection: NO")

        if len(faces):
            for (x,y,w,h) in faces:
                cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
            
        # image segmentation
        cv2.imshow('result', frame)
        cv2.waitKey(1)

        # send status
        s.send(str(len(faces)).encode('utf-8'))

        # Receive data 
        data = s.recv(1024)[0] - 48

        # break
        if data == 0:
            return
    

def interact_module():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST,PORT))
        print('Socket created')
        return s
    
    except socket.error as err:
        print('socket error: {}'.format(err))
        exit(1)

def test_module():
    print('success to enter test module!\n')
    
    while True:
        data = s.recv(1024)[0] - 48
        s.send('3'.encode('utf-8'))
        print('test module loop data: {}'.format(data))
        if data == 0:
            print('success to return main')
            return

s = interact_module()

print('main loop enter')
while True:
    data = s.recv(1024)[0] - 48
    print('main loop data: {}, current: {}'.format(data, data))

    if data == 1:
        #test_module()
        camera_module()
        
    if data == 2:
        break

cap.release()
cv2.destroyAllWindows()
