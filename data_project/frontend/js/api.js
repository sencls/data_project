const API_BASE = '';

const api = {
    async getStation() {
        const res = await fetch(`${API_BASE}/api/station`);
        return res.json();
    },

    async getState() {
        const res = await fetch(`${API_BASE}/api/state`);
        return res.json();
    },

    async getStats() {
        const res = await fetch(`${API_BASE}/api/stats`);
        return res.json();
    },

    async getHeatmap() {
        const res = await fetch(`${API_BASE}/api/heatmap`);
        return res.json();
    },

    async getMultiObjective(from, to) {
        const res = await fetch(`${API_BASE}/api/multiobjective?from=${from}&to=${to}`);
        return res.json();
    },

    async start() {
        await fetch(`${API_BASE}/api/start`, { method: 'POST' });
    },

    async pause() {
        await fetch(`${API_BASE}/api/pause`, { method: 'POST' });
    },

    async reset() {
        await fetch(`${API_BASE}/api/reset`, { method: 'POST' });
    },

    async setSpeed(speed) {
        await fetch(`${API_BASE}/api/speed`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ speed })
        });
    },

    async loadStation(path) {
        await fetch(`${API_BASE}/api/load`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ path })
        });
    }
};
