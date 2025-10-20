from RPLCD.i2c import CharLCD
import json
import time
import paho.mqtt.client as mqtt

MQTT_BROKER = "mqtt-dashboard.com"
MQTT_PORT   = 1883
MQTT_TOPIC  = "66070291/final"

lcd = CharLCD(i2c_expander='PCF8574', address=0x27, port=1,
              cols=16, rows=2, charmap='A02', auto_linebreaks=True)

def show_text(line1, line2=""):
    """แสดงข้อความ 2 บรรทัด โดยจำกัดไม่เกิน 16 ตัวอักษร"""
    lcd.clear()
    lcd.write_string(line1[:16])
    lcd.cursor_pos = (1, 0)
    lcd.write_string(line2[:16])

def on_connect(client, userdata, flags, rc):
    print(f"[MQTT] Connected rc={rc}")
    client.subscribe(MQTT_TOPIC)
    show_text("MQTT connected", "subscribed")

def on_message(client, userdata, message):
    s = message.payload.decode('utf-8', errors='ignore').strip()
    print(f"[MQTT] {message.topic} -> {s}")

    try:
        d = json.loads(s)
        # รองรับได้ทั้งชื่อ key เดิมและใหม่
        t = d.get("temp") or d.get("temperature")
        h = d.get("humid") or d.get("humidity")

        # ถ้ามีค่าอุณหภูมิและความชื้นเป็นตัวเลข
        if isinstance(t, (int, float)) and isinstance(h, (int, float)):
            # แสดงแบบสั้นไม่ล้นจอ
            show_text(f"T:{t:.1f}C", f"H:{h:.1f}%")
            return
    except json.JSONDecodeError:
        pass

    # ถ้าไม่ใช่ JSON ก็แสดงข้อความธรรมดา
    show_text(s[:16], s[16:32])

def on_disconnect(client, userdata, rc):
    print(f"[MQTT] Disconnected rc={rc}")
    show_text("MQTT lost", "reconn...")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect

client.connect(MQTT_BROKER, MQTT_PORT, keepalive=60)
try:
    client.loop_forever()
except KeyboardInterrupt:
    pass
finally:
    lcd.clear()
    lcd.close(clear=True)
    client.disconnect()
