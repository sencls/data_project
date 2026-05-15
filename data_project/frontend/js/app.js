let renderer, heatmapRenderer;
let stationData = null;
let pollInterval = null;
let heatmapInterval = null;

async function init() {
    const mainCanvas = document.getElementById('main-canvas');
    const heatCanvas = document.getElementById('heatmap-canvas');

    renderer = new Renderer(mainCanvas);
    heatmapRenderer = new HeatmapRenderer(heatCanvas);

    await loadStation();
    startPolling();
}

async function loadStation() {
    try {
        stationData = await api.getStation();
        renderer.setStation(stationData);
        renderer.draw();
    } catch (e) {
        console.error('Failed to load station:', e);
    }
}

function startPolling() {
    if (pollInterval) clearInterval(pollInterval);
    pollInterval = setInterval(pollState, 200);
    if (heatmapInterval) clearInterval(heatmapInterval);
    heatmapInterval = setInterval(pollHeatmap, 1000);
}

async function pollState() {
    try {
        const state = await api.getState();
        updateDisplay(state);
        renderer.draw(state);
    } catch (e) {
        // silently ignore
    }
}

async function pollHeatmap() {
    try {
        const data = await api.getHeatmap();
        heatmapRenderer.render(data);
    } catch (e) {
        // silently ignore
    }
}

function updateDisplay(state) {
    document.getElementById('time-display').textContent = state.time || '00:00:00';
    document.getElementById('tick-display').textContent = `Tick: ${state.tick || 0}`;

    const statusEl = document.getElementById('status-display');
    statusEl.textContent = state.running ? '运行中' : '已暂停';
    statusEl.className = state.running ? 'status-running' : 'status-stopped';

    if (state.stats) {
        document.getElementById('stat-total').textContent = state.stats.totalGenerated || 0;
        document.getElementById('stat-completed').textContent = state.stats.totalCompleted || 0;
        document.getElementById('stat-active').textContent = state.stats.currentActive || 0;
        document.getElementById('stat-bottleneck').textContent =
            state.stats.bottleneckNodeId >= 0 ? `#${state.stats.bottleneckNodeId}` : '-';
        document.getElementById('stat-maxcong').textContent =
            ((state.stats.maxCongestion || 0) * 100).toFixed(1) + '%';
    }
}

async function startSim() {
    await api.start();
}

async function pauseSim() {
    await api.pause();
}

async function resetSim() {
    await api.reset();
    await loadStation();
}

async function setSpeed(val) {
    document.getElementById('speed-val').textContent = val;
    await api.setSpeed(parseInt(val));
}

function toggleLayer(name) {
    const checked = document.getElementById(`layer-${name}`).checked;
    if (name === 'heatmap') {
        heatmapRenderer.setVisible(checked);
    } else if (renderer) {
        renderer.layers[name] = checked;
    }
}

async function analyzePaths() {
    const from = parseInt(document.getElementById('path-from').value);
    const to = parseInt(document.getElementById('path-to').value);

    if (isNaN(from) || isNaN(to)) return;

    try {
        const solutions = await api.getMultiObjective(from, to);
        renderer.setSelectedPaths(solutions);

        const container = document.getElementById('path-results');
        container.innerHTML = '';

        for (const sol of solutions) {
            const div = document.createElement('div');
            div.className = 'path-solution';
            div.innerHTML = `<h4>${sol.description}</h4>
                <div class="metric">时间: ${sol.time.toFixed(1)}s | 距离: ${sol.distance.toFixed(1)}m</div>
                <div class="metric">拥挤度: ${sol.congestionScore.toFixed(2)} | 区域切换: ${sol.areaSwitches}次</div>
                <div class="metric">路径: ${sol.path.join(' → ')}</div>`;
            container.appendChild(div);
        }
    } catch (e) {
        console.error('Path analysis failed:', e);
    }
}

document.addEventListener('DOMContentLoaded', init);
