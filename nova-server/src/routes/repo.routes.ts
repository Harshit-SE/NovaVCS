import { Router } from 'express';
import { createRepo, getAnalytics, createCommit } from '../controllers/repo.controller';

const router = Router();
router.post('/', createRepo);
router.get('/:id/analytics', getAnalytics);
router.post('/:id/commits', createCommit);
export default router;
