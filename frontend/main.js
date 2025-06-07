function drawMap() {
    const canvas = document.getElementById('flychess-map');
    const ctx = canvas.getContext('2d');
  
    const size = 40; // 格子大小
    const rows = 15;
    const cols = 15;
  
    ctx.clearRect(0, 0, canvas.width, canvas.height);
  
    ctx.strokeStyle = '#555';
    for (let r = 0; r < rows; r++) {
      for (let c = 0; c < cols; c++) {
        ctx.strokeRect(c * size, r * size, size, size);
      }
    }
  
    ctx.fillStyle = 'yellow';
    ctx.fillRect(0, 0, size, size);
  
    ctx.fillStyle = 'red';
    ctx.fillRect((cols - 1) * size, (rows - 1) * size, size, size);
  }
  
  window.onload = drawMap;
  