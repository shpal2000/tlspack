from fastapi import FastAPI

from fastapi.responses import ORJSONResponse
from fastapi.responses import HTMLResponse

app = FastAPI()

@app.get('/', response_class=HTMLResponse)
async def root():
    response = '''
    <html>
        <head>
            <title>hello from fastapi</title>
        </head>
        <body>
            <h1>hello, world!</h1>
        </body
    </html>
    '''
    return HTMLResponse(content=response, status_code=200)

@app.get('/runs', response_class=ORJSONResponse)
async def root():
    return [{"runid" : "cps1"}, {"runid" : "bw1"}]
