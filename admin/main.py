from fastapi import Depends, FastAPI 
from typing import Optional
from pydantic import BaseModel

from functools import lru_cache

import pdb

import sqlite3
from . import crud
from . import config

from fastapi.responses import ORJSONResponse
from fastapi.responses import HTMLResponse

app = FastAPI()

@lru_cache
def get_settings():
    settings = config.Settings()
    crud.create_tables (settings.db_file)
    return settings

@app.on_event("startup")
def startup():
    print ("startup")


@app.on_event("shutdown")
def shutdown():
    print ("shutdown")

class CpsLoadParams(BaseModel):
    runid: str
    cipher: str
    cps: int

class StopLoadParams(BaseModel):
    runid: str

@app.get('/index.html', response_class=HTMLResponse)
@app.get('/index.htm', response_class=HTMLResponse)
@app.get('/index', response_class=HTMLResponse)
@app.get('/', response_class=HTMLResponse)
def root(settings: config.Settings = Depends (get_settings)):
    response = '''
     <html>
         <head>
             <title>tlspack</title>
         </head>
         <body>
             <h1>hello, from tlspack!</h1>
         </body
     </html>
     '''
    return HTMLResponse(content=response, status_code=200)

@app.post('/start_cps', response_class=ORJSONResponse)
async def start_cps(params : CpsLoadParams
                    , settings: config.Settings = Depends (get_settings)):

    crud.add_task(settings.db_file, params.runid, 'cps-run')
    return params

@app.post('/stop', response_class=ORJSONResponse)
async def stop(params : StopLoadParams
                , settings: config.Settings = Depends (get_settings)):

    crud.del_task(settings.db_file, params.runid)
    return params

@app.get('/runs', response_class=ORJSONResponse)
async def show_runs(settings: config.Settings = Depends (get_settings)):
    runs = crud.get_task_ids(settings.db_file)
    return runs