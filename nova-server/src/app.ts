import express from 'express';
import cors from 'cors';
import repoRoutes from './routes/repo.routes';

const app = express();
app.use(cors());
app.use(express.json({ limit: '10mb' }));

app.get('/api/health', (req, res) => {
  res.json({ server: 'healthy', database: 'healthy', novavcs: 'available' });
});

app.use('/api/repositories', repoRoutes);

export default app;
