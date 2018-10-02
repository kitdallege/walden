import kore
import json
# This function is called when our python module is loaded/unloaded.
# The action param is kore.MODULE_LOAD or kore.MODULE_UNLOAD respectively.
#
def onload(action):
    kore.log(kore.LOG_INFO, "python module onload called with %d!" % action)
    return kore.RESULT_OK

# Called by Kore when the parent is starting.
def kore_parent_configure():
    # Listen on an additional interface and port.
    #kore.listen("127.0.0.1", "8889", "")
    kore.log(kore.LOG_INFO, "kore_parent_configure called!")

# Called by Kore when the worker is starting.
def kore_worker_configure():
    kore.log(kore.LOG_INFO, "kore_worker_configure called!")

# NOTE: there seems to be something about 'login' thats a reserved

def serve_login(req):
    kore.log(kore.LOG_INFO, "login called!")
    req.response_header("content-type", "application/json")
    req.response(200, json.dumps({
        "code": 20000,
        'method': 'login',
        'success': True,
        'data': {'token': 'bb9400bf48694218ae6d39a3b2578df2'}
    }).encode())

def serve_info(req):
    kore.log(kore.LOG_INFO, "info called!")
    req.response_header("content-type", "application/json")
    req.response(200, json.dumps({
        "code": 20000,
        'method': 'login',
        'success': True,
        "data": {
            "roles": ["admin"],
            "name": "admin",
            "avatar": "https://wpimg.wallstcn.com/f778738c-e4f8-4870-b634-56703b4acafe.gif",
            'token': 'bb9400bf48694218ae6d39a3b2578df2'
        }
    }).encode())

def serve_logout(req):
    kore.log(kore.LOG_INFO, "logout called!")
    req.response_header("content-type", "application/json")
    req.response(200, json.dumps({
        "code": 20000,
        'method': 'logout',
        'success': True
    }).encode())

