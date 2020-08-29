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

@app.post('/start_cps', response_class=ORJSONResponse)
async def start_cps(params : CpsLoadParams
                    , settings: config.Settings = Depends (get_settings)):
    conn = sqlite3.connect(settings.db_file)
    c = conn.cursor()
    c.execute ('''INSERT INTO tasks (id, type)
                    VALUES (?, ?)''',  (params.runid, 'cps-run'))
    conn.commit()
    conn.close()
    return params

@app.post('/stop', response_class=ORJSONResponse)
async def stop(params : StopLoadParams
                , settings: config.Settings = Depends (get_settings)):
    conn = sqlite3.connect(settings.db_file)
    c = conn.cursor()
    c.execute ('''DELETE FROM tasks 
                    WHERE id = ?''', (params.runid, ) )
    conn.commit()
    conn.close()
    return params

@app.get('/runs', response_class=ORJSONResponse)
async def show_runs(settings: config.Settings = Depends (get_settings)):
    runs = []
    conn = sqlite3.connect(settings.db_file)
    c = conn.cursor()
    for row in c.execute ('''SELECT id FROM tasks'''):
        runs.append ({"runid" : row[0]})
    conn.commit()
    conn.close()
    return runs