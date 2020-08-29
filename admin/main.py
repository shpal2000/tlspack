from fastapi import FastAPI
from typing import Optional
from pydantic import BaseModel

import sqlite3
from .crud import create_tables

from fastapi.responses import ORJSONResponse
from fastapi.responses import HTMLResponse

app = FastAPI()

@app.on_event("startup")
def startup():
    print ("startup")
    create_tables ()



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
async def start_cps(params : CpsLoadParams):
    conn = sqlite3.connect('tlspack.db')
    c = conn.cursor()
    c.execute ('''INSERT INTO tasks (id, type)
                    VALUES (?, ?)''',  (params.runid, 'cps-run'))
    conn.commit()
    conn.close()
    return params

@app.post('/stop', response_class=ORJSONResponse)
async def stop(params : StopLoadParams):
    conn = sqlite3.connect('tlspack.db')
    c = conn.cursor()
    c.execute ('''DELETE FROM tasks 
                    WHERE id = ?''', (params.runid, ) )
    conn.commit()
    conn.close()
    return params

@app.get('/runs', response_class=ORJSONResponse)
async def show_runs():
    runs = []
    conn = sqlite3.connect('tlspack.db')
    c = conn.cursor()
    for row in c.execute ('''SELECT id FROM tasks'''):
        runs.append ({"runid" : row[0]})
    conn.commit()
    conn.close()
    return runs