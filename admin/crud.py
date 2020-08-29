import sqlite3

def create_tables():
    conn = sqlite3.connect('tlspack.db')
    c = conn.cursor()

    sql_query = ''' CREATE TABLE IF NOT EXISTS tasks
                        (id text PRIMARY KEY, type text NOT NULL)'''
                        
    c.execute (sql_query)
    conn.commit()
    conn.close()

