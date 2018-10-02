import kore
import datetime
def onconnect(c):
    kore.log(kore.LOG_INFO, "{0!r}: connected".format(c))

def onmessage(c, op, data):
    kore.log(kore.LOG_INFO, "conn:{0!r} op:{1!r} data:{2!r}".format(c, op, data))
    c.websocket_send(op, "{0}".format(datetime.datetime.now()).encode())

def ondisconnect(c):
    kore.log(kore.LOG_INFO, "{0!r}: disconnectin".format(c))

def ws_connect(req):
    try:
        req.websocket_handshake("onconnect", "onmessage", "ondisconnect")
    except:
        req.response(500, b'')

