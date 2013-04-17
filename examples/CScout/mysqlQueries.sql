Query 1:

SELECT IDS.NAME 
FROM (
    SELECT EID,FID,COUNT(FOFFSET) 
    FROM TOKENS 
    GROUP BY EID,FID
) AS A 
JOIN IDS 
ON A.EID=IDS.EID 
WHERE IDS.LSCOPE 
AND IDS.ORDINARY 
AND NOT IDS.READONLY 
AND NOT IDS.FUN 
AND NOT IDS.UNUSED 
AND NOT IDS.CSCOPE 
GROUP BY A.EID 
HAVING COUNT(A.FID)=1 
ORDER BY IDS.NAME;


Query 2:

SELECT DISTINCT NAME FROM (
       SELECT FID FROM (
       	      SELECT EID FROM IDS 
	      WHERE LSCOPE 
	      AND UNUSED 
	      AND NOT READONLY
	      ) AS U 
       LEFT JOIN TOKENS 
       ON TOKENS.EID=U.EID
       ) AS UNUSED 
LEFT JOIN FILES 
ON FILES.FID=UNUSED.FID 
ORDER BY NAME;

Query 3:

SELECT FUNCTIONS.NAME 
FROM FUNCTIONS 
JOIN FILES 
ON FILES.FID=FUNCTIONS.FID 
WHERE NOT RO 
AND FANIN=0 
ORDER BY FUNCTIONS.NAME;