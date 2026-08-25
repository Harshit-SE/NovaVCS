import app from './app';
import { logger } from './utils/logger';

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  logger.info(`Nova Server running on port ${PORT}`);
});
