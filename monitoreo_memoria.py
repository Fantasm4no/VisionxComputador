import psutil
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import time

APP_NAME = "./build/VisionxComputador"

def encontrar_proceso(nombre):
    for p in psutil.process_iter(['name', 'cmdline']):
        if nombre in p.info['name'] or (p.info['cmdline'] and nombre in ' '.join(p.info['cmdline'])):
            return p
    return None

tiempos = []
ram_sistema_percent = []
ram_app_mb = []
cpu_app_percent = []
cpu_sistema_percent = []

fig, axs = plt.subplots(2, 1, figsize=(10, 6))
fig.suptitle("Monitoreo en Tiempo Real - RAM y CPU")

print("🔍 Buscando proceso...")
proceso = encontrar_proceso(APP_NAME)

if not proceso:
    print(f"⚠️  La aplicación '{APP_NAME}' no está corriendo.")
    print("➡️  Ejecuta la app manualmente y vuelve a correr este script.")
    exit(1)
else:
    print(f"✅ Proceso encontrado: PID {proceso.pid}")

inicio = time.time()

def actualizar(frame):
    t = time.time() - inicio
    tiempos.append(t)

    mem = psutil.virtual_memory()
    ram_total = mem.total / (1024 * 1024)
    ram_disponible = mem.available / (1024 * 1024)
    ram_uso_sistema = mem.percent
    ram_sistema_percent.append(ram_uso_sistema)

    try:
        proceso.cpu_percent()
        time.sleep(0.1)
        ram_app = proceso.memory_info().rss / (1024 * 1024)
        cpu_app = proceso.cpu_percent(interval=None)

        ram_app_mb.append(ram_app)
        cpu_app_percent.append(cpu_app)
        cpu_sistema = psutil.cpu_percent(interval=None)
        cpu_sistema_percent.append(cpu_sistema)

        # Nuevos datos para panel informativo
        nucleos_totales = psutil.cpu_count(logical=True)
        nucleos_fisicos = psutil.cpu_count(logical=False)
        nucleos_uso_sistema = sum(1 for n in psutil.cpu_percent(percpu=True) if n > 0)
        nucleos_app_estimado = cpu_app / 100

    except psutil.NoSuchProcess:
        print("❌ El proceso terminó.")
        exit(1)

    if len(tiempos) > 60:
        tiempos.pop(0)
        ram_sistema_percent.pop(0)
        ram_app_mb.pop(0)
        cpu_app_percent.pop(0)
        cpu_sistema_percent.pop(0)

    axs[0].clear()
    axs[0].plot(tiempos, ram_sistema_percent, label="RAM Sistema (%)", color='blue')
    axs[0].plot(tiempos, ram_app_mb, label="RAM App (MB)", color='orange')
    axs[0].set_ylabel("RAM")
    axs[0].legend()
    axs[0].grid(True)

    texto = (
        f" Tiempo: {t:.1f} s\n"
        f" RAM App: {ram_app:.1f} MB\n"
        f" RAM disponible: {ram_disponible:.1f} MB\n"
        f" RAM total: {ram_total:.1f} MB\n"
        f" CPU App: {cpu_app:.1f} %\n"
        f" CPU Total: {cpu_sistema:.1f} %\n"
        f" Núcleos totales: {nucleos_totales}\n"
        f" Núcleos físicos: {nucleos_fisicos}\n"
        f" Núcleos usados (sistema): {nucleos_uso_sistema}\n"
        f" Núcleos usados por app (estimado): {nucleos_app_estimado:.2f}"
    )
    axs[0].text(
        0.01, 1.12, texto, transform=axs[0].transAxes,
        fontsize=9, va='top', ha='left', linespacing=1.4,
        bbox=dict(facecolor='white', alpha=0.8)
    )

    axs[1].clear()
    axs[1].plot(tiempos, cpu_sistema_percent, label="CPU Sistema (%)", color='green')
    axs[1].plot(tiempos, cpu_app_percent, label="CPU App (%)", color='red')
    axs[1].set_ylabel("CPU (%)")
    axs[1].set_xlabel("Tiempo (s)")
    axs[1].legend()
    axs[1].grid(True)

ani = FuncAnimation(fig, actualizar, interval=1000)
plt.tight_layout(rect=[0, 0, 1, 0.93])
plt.show()
