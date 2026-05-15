class HeatmapRenderer {
    constructor(canvas) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.visible = false;
        this.resize();
        window.addEventListener('resize', () => this.resize());
    }

    resize() {
        const container = this.canvas.parentElement;
        this.canvas.width = container.clientWidth;
        this.canvas.height = container.clientHeight;
    }

    setVisible(v) { this.visible = v; if (!v) this.clear(); }

    clear() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
    }

    render(data) {
        if (!this.visible || !data || !data.data) return;
        this.clear();

        const ctx = this.ctx;
        const { width, height, data: grid } = data;
        const cellW = this.canvas.width / width;
        const cellH = this.canvas.height / height;

        let maxVal = 0.01;
        for (let y = 0; y < height; y++) {
            for (let x = 0; x < width; x++) {
                if (grid[y][x] > maxVal) maxVal = grid[y][x];
            }
        }

        for (let y = 0; y < height; y++) {
            for (let x = 0; x < width; x++) {
                const val = grid[y][x] / maxVal;
                if (val < 0.01) continue;

                const r = Math.floor(255 * Math.min(val * 2, 1));
                const g = Math.floor(255 * Math.max(0, 1 - val * 2));
                const b = 0;
                const a = val * 0.7;

                ctx.fillStyle = `rgba(${r}, ${g}, ${b}, ${a})`;
                ctx.fillRect(x * cellW, y * cellH, cellW + 1, cellH + 1);
            }
        }
    }
}
