from fastapi.testclient import TestClient

from .main import app

client = TestClient(app)

def test_start_cps():
    response = client.post('/start_cps', json={"runid" : "cps_trial", "cipher" : "AES128_SHA", "cps" : 10 })
    assert response.status_code == 200
    assert response.json() == {"runid" : "cps_trial", "cipher" : "AES128_SHA", "cps" : 10 }

    response = client.get('/runs')
    assert response.status_code == 200
    assert response.json() == [ {"runid" : "cps_trial" } ]

def test_stop():
    response = client.post('/stop', json={"runid" : "cps_trial"})
    assert response.status_code == 200
    assert response.json() ==  {"runid" : "cps_trial" }

    response = client.get('/runs')
    assert response.status_code == 200
    assert response.json() == []
