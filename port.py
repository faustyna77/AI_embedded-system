import os
from influxdb_client import InfluxDBClient, Point, WritePrecision
from openai import OpenAI
from dotenv import load_dotenv
from datetime import datetime,timezone
import time 

load_dotenv()

# Inicjalizacja klienta Together
llm = OpenAI(
    base_url="https://api.together.xyz/v1",
    api_key=os.getenv("TOGETHER_API_KEY")
)

# InfluxDB
influx = InfluxDBClient(
    url=os.getenv("INFLUXDB_URL"),
    token=os.getenv("INFLUXDB_TOKEN"),
    org=os.getenv("INFLUXDB_ORG")
)
query_api = influx.query_api()
write_api = influx.write_api()

# 1. Pobierz ostatni pomiar (np. napięcie, temperatura)
query = f'''
from(bucket: "{os.getenv("INFLUXDB_BUCKET")}")
  |> range(start: -48h)
  |> filter(fn: (r) => r._measurement == "ai_gpios" and r._field == "g2_digit")
  |> filter(fn:(r)=>r._measurement=="ai_gpios" and r._field=="g3_digit")
  |> last()
'''
print("query zawartosc",query)
result = query_api.query(org=os.getenv("INFLUXDB_ORG"), query=query)
print("result",result)


g2_digit= list(result[0].records)[0].get_value()
g3_digit=list(result[1].records)[0]
print("Odczytana wartość:", g2_digit)
print("g3 init",g3_digit)
'''
# 2. Parsuj wartość pomiaru
if result:
    g2_digit= list(result[0].records)[0].get_value()
    print("Odczytana wartość:", g2_digit)

    # 3. Zapytanie do modelu na podstawie temperatury
    prompt = f"""Wstepnie wyjscia GPIO  mikrokontrolera ESP32 maja ustawioną 
    \takią wartosc  {g2_digit} zmień to wyjscie teraz na przeciwne do tego co jest ustawione - wartośc może wyjść 0 lub 1, a ty masz tylko odpwoiedziec wypisując 0 lub wypisujac 1 i tylko tyle .
    przykładowy format odpwieidzi to np.1 i tyle bez żadnych opisów  """

    response = llm.chat.completions.create(
        model="mistralai/Mixtral-8x7B-Instruct-v0.1",
        messages=[{"role": "user", "content": prompt}]
    )

    decision = int(response.choices[0].message.content.strip().upper())
    print("Decyzja modelu:", decision)
    client=InfluxDBClient(


        url=os.getenv("INFLUXDB_URL"),
        token=os.getenv("INFLUXDB_TOKEN"),
        org=os.getenv("INFLUXDB_ORG")

        )
    
    # 4. Zapisz decyzję do InfluxDB
    point = Point("decision") \
        .field("g2_digit_decision", decision) \
        .time(datetime.now(timezone.utc),WritePrecision.NS)
    print(point.to_line_protocol())


    write_api.write(bucket=os.getenv("INFLUXDB_BUCKET"), org=os.getenv("INFLUXDB_ORG"),record=point)
    write_api.flush()
    time.sleep(2)
    print("Decyzja zapisana.")
else:
    print("Brak danych z InfluxDB.")
'''