const NODE_COLORS = {
    entrance: '#4CAF50',
    ticket: '#FF9800',
    security: '#f44336',
    gate: '#9C27B0',
    corridor: '#607D8B',
    stairs: '#795548',
    escalator: '#FF5722',
    platform: '#2196F3',
    exit: '#E91E63',
    hall: '#9E9E9E',
    transfer: '#00BCD4'
};

const NODE_RADII = {
    entrance: 14,
    ticket: 10,
    security: 12,
    gate: 12,
    corridor: 8,
    stairs: 10,
    escalator: 10,
    platform: 18,
    exit: 14,
    hall: 12,
    transfer: 10
};

class Renderer {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.station = null;
        this.scale = 1;
        this.offsetX = 60;
        this.offsetY = 40;
        this.layers = { heatmap: false, paths: true, density: true };
        this.hoveredNode = null;
        this.selectedPaths = [];
        this.passengerPositions = [];

        this.canvas.addEventListener('mousemove', (e) => this.onMouseMove(e));
        this.canvas.addEventListener('click', (e) => this.onClick(e));
        this.resize();
        window.addEventListener('resize', () => this.resize());
    }

    resize() {
        const container = this.canvas.parentElement;
        this.canvas.width = container.clientWidth;
        this.canvas.height = container.clientHeight;
        if (this.station) this.computeScale();
    }

    computeScale() {
        if (!this.station || !this.station.nodes) return;
        let minX = Infinity, maxX = -Infinity, minY = Infinity, maxY = -Infinity;
        for (const n of this.station.nodes) {
            if (n.x < minX) minX = n.x;
            if (n.x > maxX) maxX = n.x;
            if (n.y < minY) minY = n.y;
            if (n.y > maxY) maxY = n.y;
        }
        const w = maxX - minX + 100;
        const h = maxY - minY + 100;
        const sx = (this.canvas.width - 120) / w;
        const sy = (this.canvas.height - 80) / h;
        this.scale = Math.min(sx, sy, 2);
        this.offsetX = 60 - minX * this.scale + 20;
        this.offsetY = 40 - minY * this.scale + 20;
    }

    setStation(data) {
        this.station = data;
        this.nodeMap = {};
        if (data.nodes) {
            for (const n of data.nodes) {
                this.nodeMap[n.id] = n;
            }
        }
        this.computeScale();
    }

    toScreen(x, y) {
        return {
            sx: x * this.scale + this.offsetX,
            sy: y * this.scale + this.offsetY
        };
    }

    drawEdges() {
        if (!this.station || !this.station.edges) return;
        const ctx = this.ctx;
        ctx.strokeStyle = '#334';
        ctx.lineWidth = 2;

        for (const e of this.station.edges) {
            const from = this.nodeMap[e.from];
            const to = this.nodeMap[e.to];
            if (!from || !to) continue;

            const p1 = this.toScreen(from.x, from.y);
            const p2 = this.toScreen(to.x, to.y);

            ctx.beginPath();
            ctx.moveTo(p1.sx, p1.sy);
            ctx.lineTo(p2.sx, p2.sy);
            ctx.stroke();

            // 箭头
            const angle = Math.atan2(p2.sy - p1.sy, p2.sx - p1.sx);
            const mx = (p1.sx + p2.sx) / 2;
            const my = (p1.sy + p2.sy) / 2;
            ctx.beginPath();
            ctx.moveTo(mx + 5 * Math.cos(angle), my + 5 * Math.sin(angle));
            ctx.lineTo(mx - 5 * Math.cos(angle - 0.5), my - 5 * Math.sin(angle - 0.5));
            ctx.lineTo(mx - 5 * Math.cos(angle + 0.5), my - 5 * Math.sin(angle + 0.5));
            ctx.closePath();
            ctx.fillStyle = '#556';
            ctx.fill();
        }
    }

    drawNodes(nodeStates) {
        if (!this.station || !this.station.nodes) return;
        const ctx = this.ctx;
        const stateMap = {};

        if (nodeStates) {
            for (const ns of nodeStates) {
                stateMap[ns.id] = ns;
            }
        }

        for (const n of this.station.nodes) {
            const p = this.toScreen(n.x, n.y);
            const color = NODE_COLORS[n.type] || '#666';
            const radius = (NODE_RADII[n.type] || 10) * Math.min(this.scale, 1.5);
            const state = stateMap[n.id];

            // 密度光晕
            if (this.layers.density && state && state.congestion > 0.3) {
                const glowRadius = radius + 10 * state.congestion;
                const gradient = ctx.createRadialGradient(p.sx, p.sy, radius, p.sx, p.sy, glowRadius);
                const alpha = state.congestion * 0.6;
                gradient.addColorStop(0, `rgba(255, 68, 68, ${alpha})`);
                gradient.addColorStop(1, 'rgba(255, 68, 68, 0)');
                ctx.beginPath();
                ctx.arc(p.sx, p.sy, glowRadius, 0, Math.PI * 2);
                ctx.fillStyle = gradient;
                ctx.fill();
            }

            // 节点本体
            ctx.beginPath();
            ctx.arc(p.sx, p.sy, radius, 0, Math.PI * 2);
            ctx.fillStyle = color;
            ctx.fill();
            ctx.strokeStyle = '#fff';
            ctx.lineWidth = 1.5;
            ctx.stroke();

            // ID 标签
            ctx.fillStyle = '#fff';
            ctx.font = `${Math.max(9, 10 * Math.min(this.scale, 1.2))}px monospace`;
            ctx.textAlign = 'center';
            ctx.textBaseline = 'middle';
            ctx.fillText(n.id, p.sx, p.sy);

            // 名称标签
            if (n.name && radius > 8) {
                ctx.fillStyle = '#ccc';
                ctx.font = `${Math.max(8, 9 * Math.min(this.scale, 1))}px sans-serif`;
                ctx.textAlign = 'center';
                ctx.textBaseline = 'top';
                ctx.fillText(n.name, p.sx, p.sy + radius + 3);
            }
        }
    }

    drawPassengers(passengers) {
        if (!passengers) return;
        const ctx = this.ctx;
        this.passengerPositions = [];

        const stateColors = {
            entering: '#76FF03',
            buying_ticket: '#FFD600',
            security_check: '#FF6D00',
            at_gate: '#AA00FF',
            heading_to_platform: '#00B0FF',
            waiting_train: '#00E5FF',
            boarding: '#1DE9B6',
            exiting: '#FF1744',
            done: '#757575'
        };

        const stateLabels = {
            entering: '进站',
            buying_ticket: '购票',
            security_check: '安检',
            at_gate: '闸机',
            heading_to_platform: '前往站台',
            waiting_train: '候车',
            boarding: '上车',
            exiting: '出站',
            done: '完成'
        };

        for (const p of passengers) {
            const sp = this.toScreen(p.x, p.y);
            const color = stateColors[p.state] || '#fff';

            // Outer glow for visibility
            ctx.beginPath();
            ctx.arc(sp.sx, sp.sy, 5, 0, Math.PI * 2);
            ctx.fillStyle = color;
            ctx.globalAlpha = 0.4;
            ctx.fill();
            ctx.globalAlpha = 1;

            // Inner dot
            ctx.beginPath();
            ctx.arc(sp.sx, sp.sy, 3, 0, Math.PI * 2);
            ctx.fillStyle = color;
            ctx.fill();

            this.passengerPositions.push({
                ...p,
                sx: sp.sx,
                sy: sp.sy,
                stateLabel: stateLabels[p.state] || p.state
            });
        }
    }

    drawPaths(solutions) {
        if (!solutions || !this.station) return;
        const ctx = this.ctx;
        const colors = ['#00ff88', '#ff8800', '#00aaff'];

        for (let i = 0; i < solutions.length; i++) {
            const sol = solutions[i];
            if (!sol.path || sol.path.length < 2) continue;

            ctx.strokeStyle = colors[i % colors.length];
            ctx.lineWidth = 3;
            ctx.setLineDash([6, 3]);
            ctx.globalAlpha = 0.8;
            ctx.beginPath();

            for (let j = 0; j < sol.path.length; j++) {
                const node = this.nodeMap[sol.path[j]];
                if (!node) continue;
                const p = this.toScreen(node.x, node.y);
                if (j === 0) ctx.moveTo(p.sx, p.sy);
                else ctx.lineTo(p.sx, p.sy);
            }
            ctx.stroke();
            ctx.setLineDash([]);
            ctx.globalAlpha = 1;
        }
    }

    draw(state) {
        const ctx = this.ctx;
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

        // 网格背景
        ctx.strokeStyle = '#1a2332';
        ctx.lineWidth = 0.5;
        for (let x = 0; x < this.canvas.width; x += 40) {
            ctx.beginPath(); ctx.moveTo(x, 0); ctx.lineTo(x, this.canvas.height); ctx.stroke();
        }
        for (let y = 0; y < this.canvas.height; y += 40) {
            ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(this.canvas.width, y); ctx.stroke();
        }

        this.drawEdges();
        if (this.layers.paths && this.selectedPaths.length > 0) {
            this.drawPaths(this.selectedPaths);
        }
        this.drawNodes(state ? state.nodes : null);
        this.drawPassengers(state ? state.passengers : null);
    }

    onMouseMove(e) {
        const rect = this.canvas.getBoundingClientRect();
        const mx = e.clientX - rect.left;
        const my = e.clientY - rect.top;
        const tooltip = document.getElementById('tooltip');

        // 1. Check passenger hover first (they're on top)
        let hoveredPassenger = null;
        for (const pp of this.passengerPositions) {
            const dist = Math.sqrt((mx - pp.sx) ** 2 + (my - pp.sy) ** 2);
            if (dist < 8) {
                hoveredPassenger = pp;
                break;
            }
        }

        if (hoveredPassenger) {
            const p = hoveredPassenger;
            tooltip.style.display = 'block';
            tooltip.style.left = (mx + 15) + 'px';
            tooltip.style.top = (my - 10) + 'px';
            tooltip.innerHTML = `
                <div class="tt-title">Passenger #${p.id}</div>
                <div class="tt-row">状态: <span>${p.stateLabel || p.state}</span></div>
                <div class="tt-row">速度: <span>${(p.speed || 0).toFixed(2)}</span></div>
                <div class="tt-row">耐心度: <span>${(p.patience || 0).toFixed(2)}</span></div>
                <div class="tt-row">熟悉度: <span>${(p.familiarity || 0).toFixed(2)}</span></div>
                <div class="tt-row">行程: <span>${p.travelTime || 0}s</span></div>
                <div class="tt-row">等待: <span>${p.waitTime || 0}s</span></div>
                <div class="tt-row">起点: <span>#${p.origin || '?'}</span> → 终点: <span>#${p.dest || '?'}</span></div>`;
            this.hoveredNode = null;
            return;
        }

        // 2. Check node hover
        this.hoveredNode = null;
        if (this.station && this.station.nodes) {
            for (const n of this.station.nodes) {
                const p = this.toScreen(n.x, n.y);
                const r = (NODE_RADII[n.type] || 10) * Math.min(this.scale, 1.5) + 4;
                if (Math.abs(mx - p.sx) < r && Math.abs(my - p.sy) < r) {
                    this.hoveredNode = n;
                    break;
                }
            }
        }

        if (this.hoveredNode) {
            const n = this.hoveredNode;
            tooltip.style.display = 'block';
            tooltip.style.left = (mx + 15) + 'px';
            tooltip.style.top = (my - 10) + 'px';
            tooltip.innerHTML = `<div class="tt-title">${n.name || n.type} #${n.id}</div>
                <div class="tt-row">类型: <span>${n.type}</span></div>
                <div class="tt-row">容量: <span>${n.capacity}</span></div>
                <div class="tt-row">楼层: <span>B${Math.abs(n.floor) + 1}</span></div>`;
        } else {
            tooltip.style.display = 'none';
        }
    }

    onClick(e) {
        if (this.hoveredNode) {
            const n = this.hoveredNode;
            console.log('Clicked node:', n.id, n.name, n.type);
        }
    }

    setSelectedPaths(solutions) {
        this.selectedPaths = solutions;
    }
}
